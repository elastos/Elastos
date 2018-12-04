package net

import (
	"math/rand"
	"testing"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

func TestPeers_GetSyncPeer(t *testing.T) {
	peers := newPeers(new(Peer))
	addPeer := func() {
	ADD:
		peer := new(Peer)
		peer.id = rand.Uint64()
		peer.height = rand.Uint64()
		peer.SetState(p2p.ESTABLISH)
		peers.AddPeer(peer)
		if peers.PeersCount() < 10 {
			goto ADD
		}
	}

	done := make(chan struct{})

	go func() {
		for {
			addPeer()
		}
	}()

	go func() {
		count := 0
		for {
		NEXT:
			peer := peers.GetSyncPeer()
			if peer == nil {
				goto NEXT
			}
			t.Logf("peers.GetSyncPeer() %v", peer)
			peers.RemovePeer(peer.ID())
			count++
			if count > 100 {
				done <- struct{}{}
			}
		}
	}()

	<-done
}
