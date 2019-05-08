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

	priKey, pubKey, err := crypto.GenerateKeyPair()
	assert.NoError(t, err)
	pk1, err := pubKey.EncodePoint(true)
	assert.NoError(t, err)

	active := make(chan struct{})
	routes := New(&Config{
		PID:        pk1,
		Addr:       "localhost",
		TimeSource: blockchain.NewMedianTime(),
		Sign: func(data []byte) (signature []byte) {
			signature, err = crypto.Sign(priKey, data)
			return
		},
		RelayAddr: func(iv *msg.InvVect, data interface{}) {
			active <- struct{}{}
		},
		OnCipherAddr: func(pid peer.PID, addr []byte) {},
	})
	routes.Start()

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
		var pid1, pid2 peer.PID
		_, pubKey, err := crypto.GenerateKeyPair()
		assert.NoError(t, err)
		pk2, err := pubKey.EncodePoint(true)
		assert.NoError(t, err)
		copy(pid1[:], pk1)
		copy(pid2[:], pk2)
		peers := []peer.PID{pid1, pid2}
		for i := 0; true; i++ {
			if i%2 == 1 {
				routes.queue <- peersMsg{peers: peers}
			} else {
				routes.queue <- peersMsg{peers: peers[1:]}
			}
			active <- struct{}{}
		}
	}()

	quit := make(chan struct{})
	time.AfterFunc(time.Minute, func() {
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

		case <-checkTimer.C:
			t.Fatal("routes deadlock")

		case <-quit:
			break out
		}
	}
}
