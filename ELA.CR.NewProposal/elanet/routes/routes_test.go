package routes

import (
	"crypto/rand"
	"fmt"
	"net"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	dp "github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	ep "github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

// iPeer fakes a server.IPeer for test.
type iPeer struct {
	*peer.Peer
}

func (p *iPeer) ToPeer() *peer.Peer {
	return p.Peer
}

func (p *iPeer) AddBanScore(persistent, transient uint32, reason string) {}

func (p *iPeer) BanScore() uint32 { return 0 }

// mockPeer creates a fake elanet.Peer instance.
func mockPeer(p *peer.Peer) *ep.Peer {
	return ep.New(&iPeer{p}, &ep.Listeners{
		OnGetData: func(p *ep.Peer, msg *msg.GetData) {},
	})
}

func makeEmptyMessage(cmd string) (m p2p.Message, e error) {
	switch cmd {
	case p2p.CmdReject:
		m = &msg.Reject{}
	case p2p.CmdGetData:
		m = &msg.GetData{}
	}
	return m, nil
}

func mockRemotePeer(port uint16, pc chan<- *peer.Peer, mc chan<- p2p.Message) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		Magic:            123123,
		DefaultPort:      port,
		MakeEmptyMessage: makeEmptyMessage,
		BestHeight:       func() uint64 { return 0 },
		IsSelfConnection: func(net.IP, int, uint64) bool { return false },
		GetVersionNonce:  func() uint64 { return 0 },
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m.(type) {
			case *msg.Version:
				pc <- peer
			default:
				mc <- m
			}
		},
	}

	listen, err := net.Listen("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		return err
	}
	go func() {
		for {
			conn, err := listen.Accept()
			if err != nil {
				fmt.Printf("%s can not accept, %s", listen.Addr(), err)
				return
			}

			p := peer.NewInboundPeer(cfg)
			p.AssociateConnection(conn)

			go func() {
				p.WaitForDisconnect()
				pc <- p
			}()
		}
	}()
	return nil
}

func mockInboundPeer(addr string, pc chan<- *peer.Peer, mc chan<- p2p.Message) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		Magic:            123123,
		DefaultPort:      12345,
		MakeEmptyMessage: makeEmptyMessage,
		BestHeight:       func() uint64 { return 0 },
		IsSelfConnection: func(net.IP, int, uint64) bool { return false },
		GetVersionNonce:  func() uint64 { return 0 },
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.Version:
				pc <- peer
			default:
				mc <- m
			}
		},
	}

	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return err
	}
	p, err := peer.NewOutboundPeer(cfg, addr)
	if err != nil {
		return err
	}
	p.AssociateConnection(conn)

	go func() {
		p.WaitForDisconnect()
		pc <- p
	}()
	return nil
}

// This test is to ensure the routes message queue will never deadlock.
func TestRoutes_Messages(t *testing.T) {
	test.SkipShort(t)

	pc := make(chan *peer.Peer, 1)
	mc := make(chan p2p.Message)
	err := mockRemotePeer(20338, pc, mc)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	err = mockInboundPeer("localhost:20338", pc, mc)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	p1 := mockPeer(<-pc)
	p2 := mockPeer(<-pc)

	go func() {
		count := 0
		for {
			select {
			case m := <-mc:
				switch m.CMD() {
				case p2p.CmdGetData:
					count++
				}
			case <-time.After(time.Second):
				fmt.Printf("Message timeout, count [%d]\n", count)
				t.FailNow()
			}
		}
	}()

	priKey1, pubKey1, err := crypto.GenerateKeyPair()
	assert.NoError(t, err)
	pk1, err := pubKey1.EncodePoint(true)
	assert.NoError(t, err)

	_, pubKey2, err := crypto.GenerateKeyPair()
	assert.NoError(t, err)
	pk2, err := pubKey2.EncodePoint(true)
	assert.NoError(t, err)

	relay := make(chan struct{})
	active := make(chan struct{})
	routes := New(&Config{
		PID:        pk1,
		Addr:       "localhost",
		TimeSource: blockchain.NewMedianTime(),
		Sign: func(data []byte) (signature []byte) {
			signature, err = crypto.Sign(priKey1, data)
			return
		},
		IsCurrent: func() bool { return true },
		RelayAddr: func(iv *msg.InvVect, data interface{}) {
			relay <- struct{}{}
		},
		OnCipherAddr: func(pid dp.PID, addr []byte) {},
	})
	routes.Start()

	// Trigger peers change continuously.
	go func() {
		var pid1, pid2 dp.PID
		copy(pid1[:], pk1)
		copy(pid2[:], pk2)
		peers := []dp.PID{pid1, pid2}
		for i := 0; i < 1; i++ {
			if i%2 == 0 {
				routes.queue <- peersMsg{peers: peers}
			} else {
				routes.queue <- peersMsg{peers: peers[1:]}
			}
		}
	}()

	// Trigger NewPeer and DonePeer continuously.
	go func() {
		routes.NewPeer(p1)
		routes.NewPeer(p2)
		for i := 0; true; i++ {
			p := &ep.Peer{}
			routes.NewPeer(p)
			active <- struct{}{}
			if i > 5 {
				routes.DonePeer(p)
				active <- struct{}{}
			}
		}
	}()

	// Trigger address announce continuously.
	go func() {
		for {
			routes.AnnounceAddr()
		}
	}()

	// Queue getData message continuously.
	go func() {
		for {
			inv := msg.NewGetData()
			hash := common.Uint256{}
			rand.Read(hash[:])
			inv.AddInvVect(msg.NewInvVect(msg.InvTypeAddress, &hash))
			routes.OnGetData(p1, inv)
			active <- struct{}{}
		}
	}()

	// Queue inv message continuously.
	go func() {
		for {
			inv := msg.NewInv()
			hash := common.Uint256{}
			rand.Read(hash[:])
			inv.AddInvVect(msg.NewInvVect(msg.InvTypeAddress, &hash))
			routes.QueueInv(p2, inv)
			active <- struct{}{}
		}
	}()

	relays := 0
	quit := make(chan struct{})
	time.AfterFunc(2*time.Minute, func() {
		close(quit)
	})
	checkTimer := time.NewTimer(time.Second)
out:
	for {
		select {
		case <-active:
			time.Sleep(time.Millisecond)
			checkTimer.Stop()
			checkTimer.Reset(time.Second)

		case <-relay:
			relays++
			if relays > 4 {
				t.Logf("routes relay(%d) too frequent", relays)
			}

		case <-checkTimer.C:
			t.Fatalf("routes deadlock")

		case <-quit:
			break out
		}
	}

	assert.Equal(t, 4, relays)
}
