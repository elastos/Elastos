// Copyright (c) 2015-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package peer_test

import (
	"container/list"
	"errors"
	"fmt"
	"io"
	"math/rand"
	"net"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

func makeEmptyMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)
	case p2p.CmdVerAck:
		message = new(msg.VerAck)
	default:
		err = fmt.Errorf("unknown message type %s", cmd)
	}

	return message, err
}

// conn mocks a network connection by implementing the net.Conn interface.  It
// is used to test peer connection without actually opening a network
// connection.
type conn struct {
	io.Reader
	io.Writer
	io.Closer

	// local network, address for the connection.
	lnet, laddr string

	// remote network, address for the connection.
	rnet, raddr string
}

// LocalAddr returns the local address for the connection.
func (c conn) LocalAddr() net.Addr {
	return &addr{c.lnet, c.laddr}
}

// Remote returns the remote address for the connection.
func (c conn) RemoteAddr() net.Addr {
	return &addr{c.rnet, c.raddr}
}

// Close handles closing the connection.
func (c conn) Close() error {
	if c.Closer == nil {
		return nil
	}
	return c.Closer.Close()
}

func (c conn) SetDeadline(t time.Time) error      { return nil }
func (c conn) SetReadDeadline(t time.Time) error  { return nil }
func (c conn) SetWriteDeadline(t time.Time) error { return nil }

// addr mocks a network address
type addr struct {
	net, address string
}

func (m addr) Network() string { return m.net }
func (m addr) String() string  { return m.address }

// pipe turns two mock connections into a full-duplex connection similar to
// net.Pipe to allow pipe's with (fake) addresses.
func pipe(c1, c2 *conn) (*conn, *conn) {
	r1, w1 := io.Pipe()
	r2, w2 := io.Pipe()

	c1.Writer = w1
	c1.Closer = w1
	c2.Reader = r1
	c1.Reader = r2
	c2.Writer = w2
	c2.Closer = w2

	return c1, c2
}

// peerStats holds the expected peer stats used for testing peer.
type peerStats struct {
	wantServices        uint64
	wantProtocolVersion uint32
	wantConnected       bool
	wantVersionKnown    bool
	wantVerAckReceived  bool
	wantBestHeight      uint32
	wantStartingHeight  uint32
	wantLastPingTime    time.Time
	wantLastPingMicros  int64
	wantTimeOffset      int64
}

// testPeer tests the given peer's flags and stats
func testPeer(t *testing.T, p *peer.Peer, s peerStats) {
	if p.Services() != s.wantServices {
		t.Errorf("testPeer: wrong Services - got %v, want %v", p.Services(), s.wantServices)
		return
	}

	if !p.LastPingTime().Equal(s.wantLastPingTime) {
		t.Errorf("testPeer: wrong LastPingTime - got %v, want %v", p.LastPingTime(), s.wantLastPingTime)
		return
	}

	if p.LastPingMicros() != s.wantLastPingMicros {
		t.Errorf("testPeer: wrong LastPingMicros - got %v, want %v", p.LastPingMicros(), s.wantLastPingMicros)
		return
	}

	if p.VerAckReceived() != s.wantVerAckReceived {
		t.Errorf("testPeer: wrong VerAckReceived - got %v, want %v", p.VerAckReceived(), s.wantVerAckReceived)
		return
	}

	if p.VersionKnown() != s.wantVersionKnown {
		t.Errorf("testPeer: wrong VersionKnown - got %v, want %v", p.VersionKnown(), s.wantVersionKnown)
		return
	}

	if p.ProtocolVersion() != s.wantProtocolVersion {
		t.Errorf("testPeer: wrong ProtocolVersion - got %v, want %v", p.ProtocolVersion(), s.wantProtocolVersion)
		return
	}

	if p.Height() != s.wantBestHeight {
		t.Errorf("testPeer: wrong BestHeight - got %v, want %v", p.Height(), s.wantBestHeight)
		return
	}

	// Allow for a deviation of 1s, as the second may tick when the message is
	// in transit and the protocol doesn't support any further precision.
	if p.TimeOffset() != s.wantTimeOffset && p.TimeOffset() != s.wantTimeOffset-1 {
		t.Errorf("testPeer: wrong TimeOffset - got %v, want %v or %v", p.TimeOffset(),
			s.wantTimeOffset, s.wantTimeOffset-1)
		return
	}

	if p.StartingHeight() != s.wantStartingHeight {
		t.Errorf("testPeer: wrong StartingHeight - got %v, want %v", p.StartingHeight(), s.wantStartingHeight)
		return
	}

	if p.Connected() != s.wantConnected {
		t.Errorf("testPeer: wrong Connected - got %v, want %v", p.Connected(), s.wantConnected)
		return
	}

	stats := p.StatsSnapshot()

	if p.ID() != stats.ID {
		t.Errorf("testPeer: wrong ID - got %v, want %v", p.ID(), stats.ID)
		return
	}

	if p.Addr() != stats.Addr {
		t.Errorf("testPeer: wrong Addr - got %v, want %v", p.Addr(), stats.Addr)
		return
	}

	if p.LastSend() != stats.LastSend {
		t.Errorf("testPeer: wrong LastSend - got %v, want %v", p.LastSend(), stats.LastSend)
		return
	}

	if p.LastRecv() != stats.LastRecv {
		t.Errorf("testPeer: wrong LastRecv - got %v, want %v", p.LastRecv(), stats.LastRecv)
		return
	}
}

// TestPeerConnection tests connection between inbound and outbound peers.
func TestPeerConnection(t *testing.T) {
	test.SkipShort(t)
	verack := make(chan struct{})
	var makeMessage p2p.MakeEmptyMessage = func(cmd string) (p2p.Message, error) {
		switch cmd {
		case p2p.CmdVerAck:
			verack <- struct{}{}
		}
		return makeEmptyMessage(cmd)
	}
	var messageFunc peer.MessageFunc = func(peer *peer.Peer, message p2p.Message) {
		switch message.(type) {
		case *msg.VerAck:
			verack <- struct{}{}
		}
	}
	peer1Cfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  pact.EBIP001Version,
		Services:         0,
		DisableRelayTx:   true,
		HostToNetAddress: nil,
		MakeEmptyMessage: makeMessage,
		BestHeight: func() uint64 {
			return 0
		},
		IsSelfConnection: func(ip net.IP, port int, nonce uint64) bool {
			return false
		},
		GetVersionNonce: func() uint64 {
			return rand.Uint64()
		},
		MessageFunc: messageFunc,
	}
	peer2Cfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  pact.EBIP001Version,
		Services:         1,
		DisableRelayTx:   true,
		HostToNetAddress: nil,
		MakeEmptyMessage: makeMessage,
		BestHeight: func() uint64 {
			return 0
		},
		IsSelfConnection: func(ip net.IP, port int, nonce uint64) bool {
			return false
		},
		GetVersionNonce: func() uint64 {
			return rand.Uint64()
		},
		MessageFunc: messageFunc,
	}

	wantStats1 := peerStats{
		wantServices:        0,
		wantProtocolVersion: pact.EBIP001Version,
		wantConnected:       true,
		wantVersionKnown:    true,
		wantVerAckReceived:  true,
		wantLastPingTime:    time.Time{},
		wantLastPingMicros:  int64(0),
		wantTimeOffset:      int64(0),
	}
	wantStats2 := peerStats{
		wantServices:        1,
		wantProtocolVersion: pact.EBIP001Version,
		wantConnected:       true,
		wantVersionKnown:    true,
		wantVerAckReceived:  true,
		wantLastPingTime:    time.Time{},
		wantLastPingMicros:  int64(0),
		wantTimeOffset:      int64(0),
	}

	tests := []struct {
		name  string
		setup func() (*peer.Peer, *peer.Peer, error)
	}{
		{
			"basic handshake",
			func() (*peer.Peer, *peer.Peer, error) {
				inConn, outConn := pipe(
					&conn{raddr: "10.0.0.1:8333"},
					&conn{raddr: "10.0.0.2:8333"},
				)
				inPeer := peer.NewInboundPeer(peer1Cfg)
				inPeer.AssociateConnection(inConn)

				outPeer, err := peer.NewOutboundPeer(peer2Cfg, "10.0.0.2:8333")
				if err != nil {
					return nil, nil, err
				}
				outPeer.AssociateConnection(outConn)

				for i := 0; i < 2; i++ {
					select {
					case <-verack:
					case <-time.After(time.Second):
						return nil, nil, errors.New("verack timeout")
					}
				}
				return inPeer, outPeer, nil
			},
		},
		{
			"socks proxy",
			func() (*peer.Peer, *peer.Peer, error) {
				inConn, outConn := pipe(
					&conn{raddr: "10.0.0.1:8333"},
					&conn{raddr: "10.0.0.2:8333"},
				)
				inPeer := peer.NewInboundPeer(peer1Cfg)
				inPeer.AssociateConnection(inConn)

				outPeer, err := peer.NewOutboundPeer(peer2Cfg, "10.0.0.2:8333")
				if err != nil {
					return nil, nil, err
				}
				outPeer.AssociateConnection(outConn)

				for i := 0; i < 2; i++ {
					select {
					case <-verack:
					case <-time.After(time.Second):
						return nil, nil, errors.New("verack timeout")
					}
				}
				return inPeer, outPeer, nil
			},
		},
	}
	t.Logf("Running %d tests", len(tests))
	for i, test := range tests {
		inPeer, outPeer, err := test.setup()
		if err != nil {
			t.Errorf("TestPeerConnection setup #%d: unexpected err %v", i, err)
			return
		}
		testPeer(t, inPeer, wantStats2)
		testPeer(t, outPeer, wantStats1)

		inPeer.Disconnect()
		outPeer.Disconnect()
		inPeer.WaitForDisconnect()
		outPeer.WaitForDisconnect()
	}
}

// Tests that the node disconnects from peers with an unsupported protocol
// version.
func TestUnsupportedVersionPeer(t *testing.T) {
	test.SkipShort(t)
	verNonce := rand.Uint64()
	peerCfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  0,
		Services:         0,
		DisableRelayTx:   true,
		HostToNetAddress: nil,
		MakeEmptyMessage: makeEmptyMessage,
		BestHeight: func() uint64 {
			return 0
		},
		IsSelfConnection: func(ip net.IP, port int, nonce uint64) bool {
			return nonce == verNonce
		},
		GetVersionNonce: func() uint64 {
			return verNonce
		},
	}

	localConn, remoteConn := pipe(
		&conn{laddr: "10.0.0.1:8333", raddr: "10.0.0.2:8333"},
		&conn{laddr: "10.0.0.2:8333", raddr: "10.0.0.1:8333"},
	)

	p, err := peer.NewOutboundPeer(peerCfg, "10.0.0.1:8333")
	if err != nil {
		t.Fatalf("NewOutboundPeer: unexpected err - %v\n", err)
	}
	p.AssociateConnection(localConn)

	// Read outbound messages to peer into a channel
	outboundMessages := make(chan p2p.Message)
	go func() {
		for {
			msg, err := p2p.ReadMessage(
				remoteConn,
				peerCfg.Magic,
				p2p.ReadMessageTimeOut,
				makeEmptyMessage,
			)
			if err == io.EOF {
				close(outboundMessages)
				return
			}
			if err != nil {
				t.Errorf("Error reading message from local node: %v\n", err)
				return
			}

			outboundMessages <- msg
		}
	}()

	// Read version message sent to remote peer
	select {
	case omsg := <-outboundMessages:
		if _, ok := omsg.(*msg.Version); !ok {
			t.Fatalf("Expected version message, got [%s]", omsg.CMD())
		}
	case <-time.After(time.Second):
		t.Fatal("Peer did not send version message")
	}

	// Remote peer writes version message advertising invalid protocol version 1
	invalidVersionMsg := msg.NewVersion(1, 0, 0, verNonce,
		0, true)

	err = p2p.WriteMessage(
		remoteConn, peerCfg.Magic, invalidVersionMsg, p2p.WriteMessageTimeOut,
		func(m p2p.Message) (*types.DposBlock, bool) {
			msgBlock, ok := m.(*msg.Block)
			if !ok {
				return nil, false
			}
			dposBlock, ok := msgBlock.Serializable.(*types.DposBlock)
			return dposBlock, ok
		})
	if err != nil {
		t.Fatalf("p2p.WriteMessageN: unexpected err - %v\n", err)
	}

	// Expect peer to disconnect automatically
	disconnected := make(chan struct{})
	go func() {
		p.WaitForDisconnect()
		disconnected <- struct{}{}
	}()

	select {
	case <-disconnected:
		close(disconnected)
	case <-time.After(time.Second):
		t.Fatal("Peer did not automatically disconnect")
	}

	// Expect no further outbound messages from peer
	select {
	case msg, chanOpen := <-outboundMessages:
		if chanOpen {
			t.Fatalf("Expected no further messages, received [%s]", msg.CMD())
		}
	case <-time.After(time.Second):
		t.Fatal("Timeout waiting for remote reader to close")
	}
}

func TestSendMessage(t *testing.T) {
	test.SkipShort(t)

	writeMessageCount := 100
	writeMessageTime := time.Second * 5

	outputQueue := make(chan bool, 50)
	sendQueue := make(chan bool, 1)
	sendDoneQueue := make(chan bool, 1)
	waiQueue := make(chan bool, 1)

	pendingMsgs := list.New()

	var sendCount int
	var writeCount int
	// To avoid duplication below.
	queuePacket := func(msg bool, list *list.List, waiting bool) bool {
		if !waiting {
			sendQueue <- msg
			sendCount++
			fmt.Println("queuePacket sendQueue ", sendCount)
		} else {
			list.PushBack(msg)
			sendCount++
			fmt.Println("queuePacket list push back ", sendCount)
		}
		// we are always waiting now.
		return true
	}

	var waiting bool
	go func() {
		for {
			select {
			case msg := <-outputQueue:
				time.Sleep(time.Second / 4)
				fmt.Println("waiting: ", waiting)
				waiting = queuePacket(msg, pendingMsgs, waiting)
			case <-sendDoneQueue:
				// No longer waiting if there are no more messages
				// in the pending messages queue.
				next := pendingMsgs.Front()
				if next == nil {
					waiting = false
					continue
				}

				// Notify the outHandler about the next item to
				// asynchronously send.
				val := pendingMsgs.Remove(next)
				sendQueue <- val.(bool)
			}
		}
	}()

	go func() {
		for {
			select {
			case <-sendQueue:
				fmt.Println("write ", writeCount)
				time.Sleep(writeMessageTime)
				sendDoneQueue <- true
				writeCount++
				if writeCount == writeMessageCount {
					waiQueue <- true
				}
			}
		}
	}()

	for i := 0; i < writeMessageCount; i++ {
		fmt.Println("output message ", i+1)
		outputQueue <- true
	}

	<-waiQueue

	assert.Equal(t, writeCount, 100)
}

func TestWriteAfterTimeout(t *testing.T) {
	test.SkipShort(t)
	timeOutput := 2

	c1, err := net.ListenPacket("udp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer c1.Close()

	host, _, err := net.SplitHostPort(c1.LocalAddr().String())
	if err != nil {
		t.Fatal(err)
	}

	c2, err := net.ListenPacket(c1.LocalAddr().Network(),
		net.JoinHostPort(host, "0"))
	if err != nil {
		t.Fatal(err)
	}
	defer c2.Close()

	if err := c2.SetWriteDeadline(time.Now().Add(time.Duration(timeOutput) *
		time.Second)); err != nil {
		t.Fatalf("set write deadline error: %v", err)
	}

	var result bool
	fmt.Println("start write:", time.Now())
	for i := 0; i < timeOutput*2; i++ {
		_, err := c2.WriteTo([]byte("WRITETO TIMEOUT TEST"), c1.LocalAddr())
		if err != nil {
			result = err.(net.Error).Timeout()
			break
		}
		time.Sleep(time.Second)
	}
	fmt.Println("end write:", time.Now())

	assert.Equal(t, result, true)
}

func TestWriteToTimeout(t *testing.T) {
	test.SkipShort(t)

	// create 1G data to send.
	data := randomBytes(1024 * 1024 * 1024)
	fmt.Println("prepare data finished:", time.Now())

	ln, err := net.Listen("tcp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer ln.Close()

	c, err := net.Dial(ln.Addr().Network(), ln.Addr().String())
	if err != nil {
		t.Fatal(err)
	}
	defer c.Close()

	if err := c.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		t.Fatalf("set write deadline error: %v", err)
	}

	var result bool
	_, err = c.Write(data)
	if err != nil {
		result = err.(net.Error).Timeout()
	} else {
		result = false
	}
	assert.Equal(t, result, true)
}

func randomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}
