package net

import (
	"sync"
)

type neighbors struct {
	sync.Mutex
	list map[uint64]*Peer
}

func (ns *neighbors) Init() {
	ns.list = make(map[uint64]*Peer)
}

func (ns *neighbors) AddNeighbor(peer *Peer) {
	ns.Lock()
	defer ns.Unlock()

	ns.list[peer.ID()] = peer
}

func (ns *neighbors) DelNeighbor(id uint64) (*Peer, bool) {
	ns.Lock()
	defer ns.Unlock()

	peer, ok := ns.list[id]
	delete(ns.list, id)

	return peer, ok
}

func (ns *neighbors) GetNeighborCount() (count int) {
	ns.Lock()
	defer ns.Unlock()

	for _, n := range ns.list {
		if !n.Connected() {
			continue
		}
		count++
	}
	return count
}

func (ns *neighbors) GetNeighborPeers() []*Peer {
	ns.Lock()
	defer ns.Unlock()

	peers := make([]*Peer, 0, len(ns.list))
	for _, peer := range ns.list {
		// Skip disconnected peer
		if !peer.Connected() {
			continue
		}

		peers = append(peers, peer)
	}
	return peers
}

func (ns *neighbors) IsNeighborPeer(id uint64) bool {
	ns.Lock()
	defer ns.Unlock()

	peer, ok := ns.list[id]
	if !ok {
		return false
	}

	return peer.Connected()
}
