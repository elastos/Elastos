// Copyright (c) 2015-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package peer_test

import (
	"crypto/rand"
	"errors"
	"fmt"
	"io"
	"net"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p"
	pmsg "github.com/elastos/Elastos.ELA/p2p/msg"
)

func makeEmptyMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case msg.CmdVersion:
		message = &msg.Version{}
	case msg.CmdVerAck:
		message = &msg.VerAck{}
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
	wantConnected      bool
	wantBestHeight     uint32
	wantStartingHeight uint32
	wantLastPingTime   time.Time
	wantLastPingMicros int64
	wantTimeOffset     int64
}

// testPeer tests the given peer's flags and stats
func testPeer(t *testing.T, p *peer.Peer, s peerStats) {
	if !p.LastPingTime().Equal(s.wantLastPingTime) {
		t.Errorf("testPeer: wrong LastPingTime - got %v, want %v", p.LastPingTime(), s.wantLastPingTime)
		return
	}

	if p.LastPingMicros() != s.wantLastPingMicros {
		t.Errorf("testPeer: wrong LastPingMicros - got %v, want %v", p.LastPingMicros(), s.wantLastPingMicros)
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

func peerConfig(magic uint32, verack chan<- struct{}) *peer.Config {
	var pid peer.PID
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	return &peer.Config{
		Magic:        magic,
		PID:          pid,
		PingInterval: time.Second * 30,
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
		MakeEmptyMessage: func(cmd string) (p2p.Message, error) {
			return makeEmptyMessage(cmd)
		},
		MessageFunc: func(peer *peer.Peer, message p2p.Message) {
			switch message.(type) {
			case *msg.VerAck:
				verack <- struct{}{}
			}
		},
	}
}

// TestPeerConnection tests connection between inbound and outbound peers.
func TestPeerConnection(t *testing.T) {
	verack := make(chan struct{})
	peer1Cfg := peerConfig(123123, verack)
	peer2Cfg := peerConfig(123123, verack)

	wantStats1 := peerStats{
		wantConnected:      true,
		wantLastPingTime:   time.Time{},
		wantLastPingMicros: int64(0),
		wantTimeOffset:     int64(0),
	}
	wantStats2 := peerStats{
		wantConnected:      true,
		wantLastPingTime:   time.Time{},
		wantLastPingMicros: int64(0),
		wantTimeOffset:     int64(0),
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
		time.Sleep(time.Millisecond)
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
	var pid peer.PID
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)
	peerCfg := &peer.Config{
		Magic:        123123,
		PID:          pid,
		PingInterval: time.Second * 30,
		Sign: func(nonce []byte) []byte {
			sign, _ := crypto.Sign(priKey, nonce)
			return sign
		},
		MakeEmptyMessage: makeEmptyMessage,
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

	var nonce, target [16]byte
	rand.Read(nonce[:])
	rand.Read(target[:])
	// Remote peer writes version message advertising invalid protocol version 1
	invalidVersionMsg := msg.NewVersion(peerCfg.PID, target, nonce, 8333)

	err = p2p.WriteMessage(
		remoteConn,
		peerCfg.Magic,
		invalidVersionMsg,
		p2p.WriteMessageTimeOut,
		func(m p2p.Message) (*types.DposBlock, bool) {
			msgBlock, ok := m.(*pmsg.Block)
			if !ok {
				return nil, false
			}
			dposBlock, ok := msgBlock.Serializable.(*types.DposBlock)
			return dposBlock, ok
		},
	)
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
