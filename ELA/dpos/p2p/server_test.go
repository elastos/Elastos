package p2p

import (
	"crypto/rand"
	"fmt"
	"io"
	math "math/rand"
	"net"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/stretchr/testify/assert"
)

type message struct {
	pid common.Uint256
}

func (msg *message) CMD() string {
	return "message"
}

func (msg *message) MaxLength() uint32 {
	return 1024
}

func (msg *message) Serialize(w io.Writer) error {
	return msg.pid.Serialize(w)
}

func (msg *message) Deserialize(r io.Reader) error {
	return msg.pid.Deserialize(r)
}

func mockRemotePeer(t *testing.T, pid [32]byte, port uint16,
	pc chan<- *peer.Peer, mc chan<- p2p.Message) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  0,
		Services:         0,
		PID:              pid,
		MakeEmptyMessage: makeEmptyMessage,
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

			p.AddMessageFunc(func(peer *peer.Peer, m p2p.Message) {
				switch m := m.(type) {
				case *msg.VerAck:
					pc <- p

				case *message:
					mc <- m
					if !m.pid.IsEqual(pid) {
						t.Fatal("PID in message not match")
					}
				}
			})

			go func() {
				p.WaitForDisconnect()
				pc <- p
			}()
		}
	}()
	return nil
}

func mockInboundPeer(t *testing.T, addr PeerAddr, pc chan<- *peer.Peer,
	mc chan<- p2p.Message) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  0,
		Services:         0,
		PID:              addr.PID,
		MakeEmptyMessage: makeEmptyMessage,
	}

	conn, err := net.Dial("tcp", "localhost:20338")
	if err != nil {
		return err
	}
	p, err := peer.NewOutboundPeer(cfg, addr.Addr)
	if err != nil {
		return err
	}
	p.AssociateConnection(conn)
	p.AddMessageFunc(func(peer *peer.Peer, m p2p.Message) {
		switch m := m.(type) {
		case *msg.VerAck:
			pc <- p

		case *message:
			mc <- m
			if !m.pid.IsEqual(addr.PID) {
				t.Fatal("PID in message not match")
			}
		}
	})

	go func() {
		p.WaitForDisconnect()
		pc <- p
	}()
	return nil
}

func TestServer_ConnectPeers(t *testing.T) {
	// Start peer-to-peer server
	pid := [32]byte{}
	rand.Read(pid[:])
	server, err := NewServer(&Config{
		PID:              pid,
		MagicNumber:      123123,
		ProtocolVersion:  0,
		Services:         0,
		DefaultPort:      20338,
		MakeEmptyMessage: makeEmptyMessage,
	})
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	defer server.Stop()
	server.Start()

	peerChan := make(chan *peer.Peer)
	msgChan := make(chan p2p.Message)

	// Mock 100 remote peers and addresses.
	addrList := make([]PeerAddr, 0, 100)
	connectPeers := make(map[common.Uint256]PeerAddr)
	for i := uint16(0); i < 100; i++ {
		rand.Read(pid[:])
		port := 40000 + i
		addr := PeerAddr{PID: pid, Addr: fmt.Sprintf("localhost:%d", port)}
		addrList = append(addrList, addr)
		connectPeers[pid] = addr
		err := mockRemotePeer(t, pid, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Connect 50 peers
	server.ConnectPeers(addrList[:50])
	for i := 0; i < 50; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}

	connectedPeers := server.ConnectedPeers()
	if !assert.Equal(t, 50, len(connectedPeers)) {
		t.FailNow()
	}

	for _, p := range connectedPeers {
		index := p.ToPeer().NA().Port % 40000
		if !p.PID().IsEqual(addrList[index].PID) {
			t.Errorf("Connect peer PID not match, expect %s get %s",
				common.Uint256(addrList[index].PID), p.PID())
		}
	}
	for i := 0; i < 100; i++ {
		index := math.Intn(100)
		pid := addrList[index].PID
		err := server.SendMessageToPeer(pid, &message{pid: pid})
		if index < 50 {
			assert.NoError(t, err)
			<-msgChan
		} else {
			if !assert.Equal(t, err, ErrSendMessageFailed) {
				t.Errorf("Send message to %d>50 succeed", index)
			}
		}
	}

	// Change connect peers
	server.ConnectPeers(addrList[50:])
	for i := 0; i < 100; i++ { // 50 disconnect peers, 50 connected peers.
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}

	connectedPeers = server.ConnectedPeers()
	if !assert.Equal(t, 50, len(connectedPeers)) {
		t.FailNow()
	}
	for _, p := range connectedPeers {
		index := p.ToPeer().NA().Port % 40000
		if !p.PID().IsEqual(addrList[index].PID) {
			t.Errorf("Connect peer PID not match, expect %s got %s",
				common.Uint256(addrList[index].PID), p.PID())
		}
	}

	for i := 0; i < 100; i++ {
		index := math.Intn(100)
		pid := addrList[index].PID
		err := server.SendMessageToPeer(pid, &message{pid: pid})
		if index >= 50 {
			assert.NoError(t, err)
			<-msgChan
		} else {
			if !assert.Equal(t, err, ErrSendMessageFailed) {
				t.Errorf("Send message to %d<50 succeed", index)
			}
		}
	}

	// Connect all peers
	server.ConnectPeers(addrList)
	for i := 0; i < 50; i++ { // 50 new peers will connected.
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}

	connectedPeers = server.ConnectedPeers()
	if !assert.Equal(t, 100, len(connectedPeers)) {
		t.FailNow()
	}
	for _, p := range connectedPeers {
		index := p.ToPeer().NA().Port % 40000
		if !p.PID().IsEqual(addrList[index].PID) {
			t.Errorf("Connect peer PID not match, expect %s got %s",
				common.Uint256(addrList[index].PID), p.PID())
		}
	}

	// Mock 50 inbound peers with PID in connect list.
	for _, addr := range addrList[:50] {
		err := mockInboundPeer(t, addr, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}
	for i := 0; i < 50; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}

	connectedPeers = server.ConnectedPeers()
	if !assert.Equal(t, 150, len(connectedPeers)) {
		t.FailNow()
	}

	connectedPIDs := make(map[common.Uint256]struct{})
	for _, p := range connectedPeers {
		_, ok := connectPeers[p.PID()]
		if !assert.Equal(t, true, ok) {
			t.Errorf("Connect peer PID %s not in addr list", p.PID())
			t.FailNow()
		}
		connectedPIDs[p.PID()] = struct{}{}
	}
	if !assert.Equal(t, 100, len(connectedPIDs)) {
		t.FailNow()
	}

	// Now there are 50 inbound peers are the same PID with outbound peers,
	// only one of the peer with the PID will receive message.
	for _, addr := range addrList[:50] {
		err := server.SendMessageToPeer(addr.PID, &message{addr.PID})
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}
	for i := 0; i < 50; i++ { // 50 received messages.
		select {
		case <-msgChan:
		case <-time.After(time.Second):
			t.Fatalf("Receive message timeout")
		}
	}

	// Change connect peers, there are 50 inbound peers with the same PID as
	// 50 outbound peers, so 100 peers will be disconnected.
	server.ConnectPeers(addrList[50:])
	for i := 0; i < 100; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}

	connectedPeers = server.ConnectedPeers()
	if !assert.Equal(t, 50, len(connectedPeers)) {
		t.FailNow()
	}
}

// The peers in connect list should be reconnect when happens to disconnected.
func TestServer_PeersReconnect(t *testing.T) {
	// Start peer-to-peer server
	pid := [32]byte{}
	rand.Read(pid[:])
	server, err := NewServer(&Config{
		PID:              pid,
		MagicNumber:      123123,
		ProtocolVersion:  0,
		Services:         0,
		DefaultPort:      20338,
		MakeEmptyMessage: makeEmptyMessage,
	})
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	defer server.Stop()
	server.Start()

	peerChan := make(chan *peer.Peer)
	msgChan := make(chan p2p.Message)

	// Mock 100 remote peers and addresses.
	addrList := make([]PeerAddr, 0, 100)
	connectPeers := make(map[common.Uint256]PeerAddr)
	for i := uint16(0); i < 100; i++ {
		rand.Read(pid[:])
		port := 40000 + i
		addr := PeerAddr{PID: pid, Addr: fmt.Sprintf("localhost:%d", port)}
		addrList = append(addrList, addr)
		connectPeers[pid] = addr
		err := mockRemotePeer(t, pid, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Connect peers and disconnect them to mock unstable connection.
	server.ConnectPeers(addrList)
	for i := 0; i < 1000; i++ {
		select {
		case p := <-peerChan:
			if i < 900 { // Disconnect peers for 900 times.
				p.Disconnect()
			}

		case <-time.After(time.Minute):
			t.Fatalf("Connect peers timeout")
		}
	}
}

func makeEmptyMessage(cmd string) (m p2p.Message, e error) {
	switch cmd {
	case "message":
		m = &message{}
	}
	return m, nil
}
