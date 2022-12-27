package dns

import (
	"fmt"
	"math/rand"
	"net"
	"sync"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

var (
	nonces = sync.Map{}
)

func getNonce() uint64 {
	nonce := rand.Uint64()
	nonces.Store(nonce, nil)
	return nonce
}

func haveNonce(ip net.IP, port int, nonce uint64) bool {
	_, ok := nonces.Load(nonce)
	return ok
}

// mockInboundPeer mocks a inbound peer by the given port.
func mockInboundPeer(port int, pc chan *peer.Peer,
	ac chan *p2p.NetAddress) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  0,
		Services:         1,
		DefaultPort:      uint16(port),
		BestHeight:       bestHeight,
		MakeEmptyMessage: makeMessage,
		GetVersionNonce:  getNonce,
		IsSelfConnection: haveNonce,
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.VerAck:
				peer.QueueMessage(msg.NewGetAddr(), nil)
				pc <- peer

			case *msg.Addr:
				for _, addr := range m.AddrList {
					ac <- addr
				}
			}
		},
	}

	conn, err := net.Dial("tcp", "localhost:20338")
	if err != nil {
		return err
	}
	p, err := peer.NewOutboundPeer(cfg, fmt.Sprint("localhost:", port))
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

// TestDNS test if peers can connect to the DNS service and get addresses when
// they send a getAddr message.  This test only for human read and can not run
// automatically.
func TestDNS(t *testing.T) {
	test.SkipShort(t)
	dns, err := New("./", 123123, 20338)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	dns.Start()
	defer dns.Stop()

	port := 30000
	pc := make(chan *peer.Peer)
	ac := make(chan *p2p.NetAddress)
	for i := 0; i < 1000; i++ {
		err = mockInboundPeer(port+i, pc, ac)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	for i := 0; i < 1000; i++ {
		select {
		case <-pc:
		case <-time.After(time.Millisecond):
			t.Fatal("peer connection timeout")
		}
	}

	count := 0
	for addr := range ac {
		count++
		fmt.Printf("receive %d address %s\n", count, addr)
	}
}
