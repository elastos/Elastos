package net

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.Utility/p2p"
)

type Peers struct {
	syncPeerLock *sync.Mutex
	syncPeer     *Peer

	local     *Peer
	peersLock *sync.RWMutex
	peers     map[uint64]*Peer
}

func newPeers(localPeer *Peer) *Peers {
	peers := new(Peers)
	peers.local = localPeer
	peers.syncPeerLock = new(sync.Mutex)
	peers.peersLock = new(sync.RWMutex)
	peers.peers = make(map[uint64]*Peer)
	return peers
}

func (p *Peers) Local() *Peer {
	return p.local
}

func (p *Peers) AddPeer(peer *Peer) {
	p.peersLock.Lock()
	defer p.peersLock.Unlock()

	p.peers[peer.ID()] = peer
}

func (p *Peers) Exist(peer *Peer) bool {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	_, ok := p.peers[peer.ID()]
	return ok
}

func (p *Peers) RemovePeer(id uint64) (*Peer, bool) {
	p.peersLock.Lock()
	defer p.peersLock.Unlock()

	if p.syncPeer != nil && id == p.syncPeer.ID() {
		p.syncPeer = nil
	}

	peer, ok := p.peers[id]
	delete(p.peers, id)

	return peer, ok
}

func (p *Peers) PeersCount() int {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	return len(p.peers)
}

func (p *Peers) ConnectedPeers() []*Peer {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	peers := make([]*Peer, 0, len(p.peers))
	for _, v := range p.peers {
		peers = append(peers, v)
	}
	return peers
}

func (p *Peers) EstablishedPeer(id uint64) bool {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	peer, ok := p.peers[id]
	if !ok {
		return false
	}

	return peer.State() == ESTABLISH
}

func (p *Peers) GetBestPeer() *Peer {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	return p.getBestPeer()
}

func (p *Peers) getBestPeer() *Peer {
	var bestPeer *Peer
	for _, peer := range p.peers {

		// Skip unestablished peer
		if peer.State() != ESTABLISH {
			continue
		}

		if bestPeer == nil {
			bestPeer = peer
			continue
		}

		if peer.height > bestPeer.height {
			bestPeer = peer
		}
	}

	return bestPeer
}

func (p *Peers) Broadcast(msg Message) {
	p.peersLock.RLock()
	defer p.peersLock.RUnlock()

	for _, peer := range p.peers {

		// Skip unestablished peer
		if peer.State() != ESTABLISH {
			continue
		}

		// Skip non relay peer
		if peer.Relay() == 0 {
			continue
		}

		peer.Send(msg)
	}
}

func (p *Peers) SetSyncPeer(peer *Peer) {
	p.syncPeerLock.Lock()
	defer p.syncPeerLock.Unlock()

	p.syncPeer = peer
}

func (p *Peers) GetSyncPeer() *Peer {
	p.syncPeerLock.Lock()
	defer p.syncPeerLock.Unlock()

	if p.syncPeer == nil {
		p.syncPeer = p.getBestPeer()
	}

	return p.syncPeer
}

func (p *Peers) IsSyncPeer(peer *Peer) bool {
	p.syncPeerLock.Lock()
	defer p.syncPeerLock.Unlock()

	if p.syncPeer == nil || peer == nil {
		return false
	}

	return p.syncPeer.ID() == peer.ID()
}
