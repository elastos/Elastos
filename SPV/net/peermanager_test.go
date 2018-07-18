package net

import (
	"math/rand"
	"net"
	"testing"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/stretchr/testify/assert"
)

func TestPeerManager_AddConnectedPeer(t *testing.T) {
	log.Init(0, 5, 10)
	pm := NewPeerManager(123456, 1024, nil, 4, 100, nil)
	addPeer := func() {
	ADD:
		peer := new(Peer)
		peer.conn = new(net.TCPConn)
		peer.id = rand.Uint64()
		peer.height = rand.Uint64()
		peer.SetState(p2p.ESTABLISH)
		rand.Read(peer.ip16[:])
		peer.port = uint16(rand.Uint32())
		pm.AddConnectedPeer(peer)
		assert.Equal(t, true, pm.cm.IsConnected(peer.Addr().String()))
		if pm.PeersCount() < 10 {
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
			peer := pm.GetSyncPeer()
			if peer == nil {
				goto NEXT
			}
			pm.OnDisconnected(peer)
			assert.Equal(t, false, pm.cm.IsConnected(peer.Addr().String()))
			count++
			if count > 100 {
				done <- struct{}{}
			}
		}
	}()

	<-done
}
