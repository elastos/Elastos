package routes

import (
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	epeer "github.com/elastos/Elastos.ELA/elanet/peer"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/utils/test"
	"github.com/stretchr/testify/assert"
)

// This test is to ensure the routes address announce will never deadlock.
func TestRoutes_announce(t *testing.T) {
	test.SkipShort(t)

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
		RelayAddr: func(iv *msg.InvVect, data interface{}) {
			relay <- struct{}{}
		},
		OnCipherAddr: func(pid peer.PID, addr []byte) {},
	})
	routes.Start()

	var pid1, pid2 peer.PID
	copy(pid1[:], pk1)
	copy(pid2[:], pk2)
	routes.queue <- peersMsg{peers: []peer.PID{pid1, pid2}}

	// Trigger NewPeer and DonePeer continuously.
	go func() {
		for i := 0; true; i++ {
			p := &epeer.Peer{}
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
		for i := 0; true; i++ {
			routes.AnnounceAddr()
			active <- struct{}{}
		}
	}()

	relays := 0
	quit := make(chan struct{})
	time.AfterFunc(2*time.Minute, func() {
		quit <- struct{}{}
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
