package p2p

import (
	"sync"
)

type Peers struct {
	sync.RWMutex
	local    *Peer
	syncPeer *Peer
	peers    map[uint64]*Peer
}

func newPeers(walletId uint64) *Peers {
	peers := new(Peers)
	peers.initLocalPeer(walletId)
	peers.peers = make(map[uint64]*Peer)
	return peers
}

func (p *Peers) initLocalPeer(id uint64) {
	p.Lock()
	defer p.Unlock()

	p.local = &Peer{
		id:       id,
		version:  PeerVersion,
		port:     SPVPeerPort,
		services: 0x00,
		relay:    0x00,
	}
}

func (p *Peers) Local() *Peer {
	p.RLock()
	defer p.RUnlock()

	return p.local
}

func (p *Peers) addConnectedPeer(peer *Peer) {
	p.Lock()
	defer p.Unlock()

	p.peers[peer.ID()] = peer
}

func (p *Peers) Exist(peer *Peer) bool {
	p.RLock()
	defer p.RUnlock()

	_, ok := p.peers[peer.ID()]
	return ok
}

func (p *Peers) RemovePeer(id uint64) (*Peer, bool) {
	p.Lock()
	defer p.Unlock()

	peer, ok := p.peers[id]
	delete(p.peers, id)

	if peer.ID() == p.syncPeer.ID() {
		p.syncPeer = nil
	}
	return peer, ok
}

func (p *Peers) PeersCount() int {
	return len(p.peers)
}

func (p *Peers) ConnectedPeers() []*Peer {
	p.RLock()
	defer p.RUnlock()

	peers := make([]*Peer, len(p.peers))
	for _, v := range p.peers {
		peers = append(peers, v)
	}
	return peers
}

func (p *Peers) EstablishedPeer(id uint64) bool {
	p.Lock()
	defer p.Unlock()

	peer, ok := p.peers[id]
	if !ok {
		return false
	}

	return peer.State() == ESTABLISH
}

func (p *Peers) GetBestPeer() *Peer {
	p.RLock()
	defer p.RLock()

	return p.getBestPeer()
}

func (p *Peers) getBestPeer() *Peer {
	var bestPeer *Peer
	for _, peer := range p.peers {

		// Skip unestablished peer
		if peer.state != ESTABLISH {
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

func (p *Peers) SetSyncPeer(peer *Peer) {
	p.Lock()
	defer p.Unlock()

	p.syncPeer = peer
}

func (p *Peers) GetSyncPeer() *Peer {
	p.RLock()
	defer p.RUnlock()

	if p.syncPeer == nil {
		p.syncPeer = p.getBestPeer()
	}
	return p.syncPeer
}

func (p *Peers) IsSyncPeer(peer *Peer) bool {
	p.Lock()
	defer p.Unlock()

	return p.isSyncPeer(peer)
}

func (p *Peers) isSyncPeer(peer *Peer) bool {
	if p.syncPeer == nil || peer == nil {
		return false
	}

	return p.syncPeer.ID() == peer.ID()
}

func (p *Peers) HasSyncPeer() bool {
	p.Lock()
	defer p.Unlock()

	return p.syncPeer == nil
}

func (p *Peers) Broadcast(msg []byte) {
	p.RLock()
	defer p.RUnlock()

	for _, peer := range p.peers {

		// Skip unestablished peer
		if peer.state != ESTABLISH {
			continue
		}

		// Skip non relay peer
		if peer.Relay() == 0 {
			continue
		}

		go peer.Send(msg)
	}
}
