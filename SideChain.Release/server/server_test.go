package server

import (
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/netsync"

	"github.com/elastos/Elastos.ELA/p2p/peer"
	svr "github.com/elastos/Elastos.ELA/p2p/server"
	"github.com/stretchr/testify/assert"
)

// iPeer fakes a server.IPeer for test.
type iPeer struct {
	*peer.Peer
}

func (p *iPeer) ToPeer() *peer.Peer {
	return p.Peer
}

func (p *iPeer) AddBanScore(persistent, transient uint32, reason string) {}

func (p *iPeer) BanScore() uint32 { return 0 }

// mockPeer creates a fake server.IPeer instance.
func mockPeer() svr.IPeer {
	return &iPeer{Peer: &peer.Peer{}}
}

// TestHandlePeerMsg tests the adding/removing peer messages.
func TestHandlePeerMsg(t *testing.T) {
	peers := make(map[svr.IPeer]*serverPeer)

	s := &server{
		syncManager: netsync.New(&netsync.Config{MaxPeers: 2}),
	}

	// New peers should be added.
	reply := make(chan struct{}, 1)
	p1, p2, p3 := mockPeer(), mockPeer(), mockPeer()
	s.handlePeerMsg(peers, newPeerMsg{p1, reply})
	<-reply
	s.handlePeerMsg(peers, newPeerMsg{p2, reply})
	<-reply

	assert.Equal(t, 2, len(peers))

	// Unknown done peer should not change peers.
	s.handlePeerMsg(peers, donePeerMsg{p3, reply})
	<-reply
	assert.Equal(t, 2, len(peers))

	// p1 should be removed.
	s.handlePeerMsg(peers, donePeerMsg{p1, reply})
	<-reply
	assert.Equal(t, 1, len(peers))

	// Same peer can not be removed twice.
	s.handlePeerMsg(peers, donePeerMsg{p1, reply})
	<-reply
	assert.Equal(t, 1, len(peers))

	// New peer can be added.
	s.handlePeerMsg(peers, newPeerMsg{p3, reply})
	<-reply
	assert.Equal(t, 2, len(peers))
}
