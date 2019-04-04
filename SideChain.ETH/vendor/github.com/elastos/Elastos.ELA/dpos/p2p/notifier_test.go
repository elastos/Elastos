package p2p

import (
	"crypto/rand"
	"fmt"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

func TestNotifier(t *testing.T) {
	test.SkipShort(t)
	// Start peer-to-peer server
	pid := peer.PID{}
	priKey, pubKey, _ := crypto.GenerateKeyPair()
	ePubKey, _ := pubKey.EncodePoint(true)
	copy(pid[:], ePubKey)

	notifyChan := make(chan NotifyFlag)
	notifier := NewNotifier(NFNetStabled|NFBadNetwork, func(flag NotifyFlag) {
		notifyChan <- flag
	})
	server, err := NewServer(&Config{
		PID:             pid,
		MagicNumber:     123123,
		ProtocolVersion: 0,
		Services:        0,
		DefaultPort:     20338,
		SignNonce: func(nonce []byte) (signature [64]byte) {
			sign, _ := crypto.Sign(priKey, nonce)
			copy(signature[:], sign)
			return signature
		},
		MakeEmptyMessage: makeEmptyMessage,
		StateNotifier:    notifier,
	})
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	defer server.Stop()
	server.Start()

	peers := 90
	portBase := 40000
	addrList := make([]PeerAddr, 0, peers)
	for i := 0; i < peers; i++ {
		rand.Read(pid[:])
		port := portBase + i
		addr := PeerAddr{PID: pid, Addr: fmt.Sprintf("localhost:%d", port)}
		addrList = append(addrList, addr)
	}
	server.ConnectPeers(addrList)

	// Mock peers not started, wait for connection timeout.
	select {
	case f := <-notifyChan:
		if !assert.Equal(t, NFBadNetwork, f) {
			t.FailNow()
		}
	case <-time.After(time.Second * 40):
		t.Fatalf("Connect peers timeout")
	}

	// Start mock peers and connect them.
	peerChan := make(chan *peer.Peer, peers)
	addrList = addrList[:0]
	for i := 0; i < peers; i++ {
		priKey, pubKey, _ := crypto.GenerateKeyPair()
		ePubKey, _ := pubKey.EncodePoint(true)
		copy(pid[:], ePubKey)
		port := portBase + i
		addr := PeerAddr{PID: pid, Addr: fmt.Sprintf("localhost:%d", port)}
		addrList = append(addrList, addr)
		err := mockRemotePeer(pid, priKey, uint16(port), peerChan, nil)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}
	server.ConnectPeers(addrList)

	// Wait for network stable notify.
	select {
	case f := <-notifyChan:
		if !assert.Equal(t, NFNetStabled, f) {
			t.FailNow()
		}
	case <-time.After(time.Second * 10):
		t.Fatalf("Connect peers timeout")
	}

	// Disconnect 31 peers to trigger bad network notify
	for i := 0; i < 31; i++ {
		p := <-peerChan
		p.Disconnect()
	}
	select {
	case f := <-notifyChan:
		if !assert.Equal(t, NFBadNetwork, f) {
			t.FailNow()
		}
	case <-time.After(time.Second * 10):
		t.Fatalf("Connect peers timeout")
	}

	// Reconnection will trigger another network stable notify.
	select {
	case f := <-notifyChan:
		if !assert.Equal(t, NFNetStabled, f) {
			t.FailNow()
		}
	case <-time.After(time.Second * 10):
		t.Fatalf("Connect peers timeout")
	}
}
