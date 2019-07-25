package p2p

import (
	"fmt"
	"io"
	"math/rand"
	"net"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

type message struct {
	pid peer.PID
}

func (msg *message) CMD() string {
	return "message"
}

func (msg *message) MaxLength() uint32 {
	return 1024
}

func (msg *message) Serialize(w io.Writer) error {
	_, err := w.Write(msg.pid[:])
	return err
}

func (msg *message) Deserialize(r io.Reader) error {
	_, err := io.ReadFull(r, msg.pid[:])
	return err
}

func mockRemotePeer(pid peer.PID, priKey []byte, port uint16,
	pc chan<- *peer.Peer, mc chan<- p2p.Message) error {

	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		PID:          pid,
		Magic:        123123,
		PingInterval: defaultPingInterval,
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
		PingNonce:        func(pid peer.PID) uint64 { return 0 },
		PongNonce:        func(pid peer.PID) uint64 { return 0 },
		MakeEmptyMessage: makeEmptyMessage,
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.VerAck:
				pc <- peer

			case *message:
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

func mockInboundPeer(addr PeerAddr, priKey []byte, pc chan<- *peer.Peer,
	mc chan<- p2p.Message) error {
	// Configure peer to act as a simnet node that offers no services.
	cfg := &peer.Config{
		PID:          addr.PID,
		Magic:        123123,
		PingInterval: defaultPingInterval,
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
		PingNonce:        func(pid peer.PID) uint64 { return 0 },
		PongNonce:        func(pid peer.PID) uint64 { return 0 },
		MakeEmptyMessage: makeEmptyMessage,
		MessageFunc: func(peer *peer.Peer, m p2p.Message) {
			switch m := m.(type) {
			case *msg.VerAck:
				pc <- peer

			case *message:
				mc <- m
			}
		},
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

	go func() {
		p.WaitForDisconnect()
		pc <- p
	}()
	return nil
}

// Test multiple servers connect to each other.
func TestServerConnections(t *testing.T) {
	test.SkipShort(t)
	// Mock 71 server config and addresses.  Why 71 ? because 71[servers]*
	// ((71-1)*2)[inbound and outbound connections] = 9940, and terminal ulimit
	// parameter is 10240, so 71 is the maximum servers can mock on my computer.
	servers := 71
	cfgs := make([]Config, 0, servers)
	peerList := make([]peer.PID, 0, servers)
	addrList := make(map[peer.PID]string)
	pid := peer.PID{}
	for i := 0; i < servers; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)

		port := 40000 + i

		cfgs = append(cfgs, Config{
			PID:         pid,
			MagicNumber: 123123,
			DefaultPort: uint16(port),
			TimeSource:  dtime.NewMedianTime(),
			Sign: func(nonce []byte) []byte {
				sign, _ := crypto.Sign(priKey, nonce)
				return sign
			},
			MakeEmptyMessage: makeEmptyMessage,
		})

		peerList = append(peerList, pid)
		addrList[pid] = fmt.Sprintf("127.0.0.1:%d", port)
	}

	// Start 71 servers.
	serverChan := make(chan *server, servers)
	doneChan := make(chan struct{})
	for _, cfg := range cfgs {
		s, err := NewServer(&cfg)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
		for _, pid := range peerList {
			s.AddAddr(pid, addrList[pid])
		}

		s.Start()
		s.ConnectPeers(peerList)
		serverChan <- s

		// There will be 70 outbound connections and 70 inbound connections
		// for each server
		go func() {
			ticker := time.NewTicker(time.Millisecond * 100)
			defer ticker.Stop()

		out:
			for {
				select {
				case <-ticker.C:
					connected := s.ConnectedCount()
					if connected >= int32(servers-1)*2 {
						break out
					}
				case <-time.After(time.Second * 10):
					t.Fatal("Server connection timeout")
				}
			}
			// Notify server connect peers completed.
			doneChan <- struct{}{}
		}()
	}
	for i := 0; i < servers; i++ {
		select {
		case <-doneChan:
		case <-time.After(time.Second * 10):
			t.Fatal("Server connect to peers timeout")
		}
	}

cleanup:
	for {
		select {
		case s := <-serverChan:
			s.Stop()
		default:
			break cleanup
		}
	}
}

func TestServer_ConnectPeers(t *testing.T) {
	test.SkipShort(t)
	// Start peer-to-peer server
	pid := peer.PID{}
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	peerList := make([]peer.PID, 0, 100)
	addrList := make(map[peer.PID]string)
	server, err := NewServer(&Config{
		PID:         pid,
		MagicNumber: 123123,
		DefaultPort: 20338,
		TimeSource:  dtime.NewMedianTime(),
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
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
	portBase := uint16(50000)
	priKeys := make([][]byte, 0, 100)
	for i := uint16(0); i < 100; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		priKeys = append(priKeys, priKey)
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := portBase + i
		peerList = append(peerList, pid)
		addrList[pid] = fmt.Sprintf("127.0.0.1:%d", port)
		server.AddAddr(pid, addrList[pid])
		err := mockRemotePeer(pid, priKey, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Connect 50 peers
	server.ConnectPeers(peerList[:50])
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
		index := p.ToPeer().NA().Port % portBase
		if !p.PID().Equal(peerList[index]) {
			t.Errorf("Connect peer PID not match, expect %s get %s",
				peerList[index], p.PID())
		}
	}
	for i := 0; i < 100; i++ {
		index := rand.Intn(100)
		pid := peerList[index]
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
	server.ConnectPeers(peerList[50:])
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
		index := p.ToPeer().NA().Port % portBase
		if !p.PID().Equal(peerList[index]) {
			t.Errorf("Connect peer PID not match, expect %s got %s",
				peerList[index], p.PID())
		}
	}

	for i := 0; i < 100; i++ {
		index := rand.Intn(100)
		pid := peerList[index]
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
	server.ConnectPeers(peerList)
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
		index := p.ToPeer().NA().Port % portBase
		if !p.PID().Equal(peerList[index]) {
			t.Errorf("Connect peer PID not match, expect %s got %s",
				peerList[index], p.PID())
		}
	}

	// Mock 50 inbound peers with PID in connect list.
	for i, pid := range peerList[:50] {
		addr := PeerAddr{
			PID:  pid,
			Addr: addrList[pid],
		}
		err := mockInboundPeer(addr, priKeys[:50][i], peerChan, msgChan)
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

	connectedPIDs := make(map[peer.PID]struct{})
	for _, p := range connectedPeers {
		_, ok := addrList[p.PID()]
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
	for _, addr := range peerList[:50] {
		err := server.SendMessageToPeer(addr, &message{addr})
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
	server.ConnectPeers(peerList[50:])
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
	test.SkipShort(t)
	// Start peer-to-peer server
	pid := peer.PID{}
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	peerList := make([]peer.PID, 0, 100)
	addrList := make(map[peer.PID]string)
	server, err := NewServer(&Config{
		PID:         pid,
		MagicNumber: 123123,
		DefaultPort: 20338,
		TimeSource:  dtime.NewMedianTime(),
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
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
	for i := uint16(0); i < 100; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := 60000 + i
		peerList = append(peerList, pid)
		addrList[pid] = fmt.Sprintf("127.0.0.1:%d", port)
		server.AddAddr(pid, addrList[pid])
		err := mockRemotePeer(pid, priKey, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Connect peers and disconnect them to mock unstable connection.
	server.ConnectPeers(peerList)
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

func TestServer_BroadcastMessage(t *testing.T) {
	test.SkipShort(t)
	// Start peer-to-peer server
	pid := peer.PID{}
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	peerList := make([]peer.PID, 0, 100)
	addrList := make(map[peer.PID]string)
	server, err := NewServer(&Config{
		PID:         pid,
		MagicNumber: 123123,
		DefaultPort: 20338,
		TimeSource:  dtime.NewMedianTime(),
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
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
	priKeys := make([][]byte, 0, 100)
	for i := uint16(0); i < 100; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		priKeys = append(priKeys, priKey)
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := 40000 + i
		peerList = append(peerList, pid)
		addrList[pid] = fmt.Sprintf("127.0.0.1:%d", port)
		server.AddAddr(pid, addrList[pid])
		err := mockRemotePeer(pid, priKey, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Mock 100 outbound peers.
	server.ConnectPeers(peerList)
	for i := 0; i < 100; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}
	if !assert.Equal(t, int32(100), server.ConnectedCount()) {
		t.FailNow()
	}

	// Mock 100 inbound peers.
	for i, pid := range peerList {
		addr := PeerAddr{
			PID:  pid,
			Addr: addrList[pid],
		}
		err := mockInboundPeer(addr, priKeys[i], peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}
	for i := 0; i < 100; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Second):
			t.Fatalf("Connect peers timeout")
		}
	}
	if !assert.Equal(t, int32(200), server.ConnectedCount()) {
		t.FailNow()
	}

	// Each PID should receive only one message.
	server.BroadcastMessage(&message{pid: pid})
	count := 0
out:
	for {
		select {
		case <-msgChan:
			count++
		case <-time.After(time.Second):
			break out
		}
	}
	if !assert.Equal(t, 100, count) {
		t.FailNow()
	}
}

func TestServer_DumpPeersInfo(t *testing.T) {
	// Start peer-to-peer server
	pid := peer.PID{}
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	peerList := make([]peer.PID, 0, 20)
	addrList := make(map[peer.PID]string)
	server, err := NewServer(&Config{
		PID:         pid,
		MagicNumber: 123123,
		DefaultPort: 20338,
		TimeSource:  dtime.NewMedianTime(),
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
		MakeEmptyMessage: makeEmptyMessage,
	})
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	defer server.Stop()
	server.Start()

	peerChan := make(chan *peer.Peer)
	msgChan := make(chan p2p.Message)

	// Mock 10 valid remote peers and addresses.
	priKeys := make([][]byte, 0, 20)
	for i := uint16(0); i < 10; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		priKeys = append(priKeys, priKey)
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := 20100 + i
		peerList = append(peerList, pid)
		addrList[pid] = fmt.Sprintf("127.0.0.1:%d", port)
		server.AddAddr(pid, addrList[pid])
		err := mockRemotePeer(pid, priKey, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Mock 10 invalid remote peers and addresses.
	for i := uint16(0); i < 5; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := 20110 + i
		peerList = append(peerList, pid)
		err := mockRemotePeer(pid, priKey, port, peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	// Wait for 10 valid outbound peers connected.
	server.ConnectPeers(peerList)
	for i := 0; i < 10; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Minute):
			t.Fatalf("Connect peers timeout")
		}
	}

	// Create 5 valid inbound peers.
	for i, pid := range peerList[:5] {
		addr := PeerAddr{
			PID:  pid,
			Addr: addrList[pid],
		}
		err := mockInboundPeer(addr, priKeys[i], peerChan, msgChan)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}
	for i := 0; i < 5; i++ {
		select {
		case <-peerChan:
		case <-time.After(time.Minute):
			t.Fatalf("Connect peers timeout")
		}
	}

	// Now there will be 5 2WayConnection peers, 5 OutboundOnly peers,
	// 5 NoneConnection peers.
	peers := server.DumpPeersInfo()
	connPeers, outPeers, nonePeers := 0, 0, 0
	for _, p := range peers {
		switch p.State {
		case CS2WayConnection:
			connPeers++
		case CSOutboundOnly:
			outPeers++
		case CSNoneConnection:
			nonePeers++
		}
	}
	if !assert.Equal(t, 5, connPeers) {
		t.FailNow()
	}
	if !assert.Equal(t, 5, outPeers) {
		t.FailNow()
	}
	if !assert.Equal(t, 5, nonePeers) {
		t.FailNow()
	}
}

func makeEmptyMessage(cmd string) (m p2p.Message, e error) {
	switch cmd {
	case p2p.CmdReject:
		m = &msg.Reject{}
	case "message":
		m = &message{}
	}
	return m, nil
}
