package sdk

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	SyncTickInterval  = time.Second * 5
	FalsePositiveRate = float32(1) / float32(1000)
)

type neighbors struct {
	sync.Mutex
	list map[uint64]*SPVPeer
}

func (ns *neighbors) init() {
	ns.list = make(map[uint64]*SPVPeer)
}

func (ns *neighbors) addNeighbor(peer *SPVPeer) {
	// Add peer to list
	ns.Lock()
	ns.list[peer.ID()] = peer
	ns.Unlock()
}

func (ns *neighbors) delNeighbor(id uint64) {
	ns.Lock()
	delete(ns.list, id)
	ns.Unlock()
}

func (ns *neighbors) getNeighborPeers() []*SPVPeer {
	ns.Lock()
	defer ns.Unlock()

	peers := make([]*SPVPeer, 0, len(ns.list))
	for _, peer := range ns.list {
		if !peer.Connected() {
			continue
		}

		peers = append(peers, peer)
	}

	return peers
}

func (ns *neighbors) getBestPeer() *SPVPeer {
	ns.Lock()
	defer ns.Unlock()
	var best *SPVPeer
	for _, peer := range ns.list {
		// Skip disconnected peer
		if !peer.Connected() {
			continue
		}

		// Init best peer
		if best == nil {
			best = peer
			continue
		}

		if peer.Height() > best.Height() {
			best = peer
		}
	}

	return best
}

type SyncManageConfig struct {
	LocalHeight func() uint32
	GetBlocks   func() *msg.GetBlocks
}

type SyncManager struct {
	config   SyncManageConfig
	syncPeer *SPVPeer
	neighbors
}

func NewSyncManager(config SyncManageConfig) *SyncManager {
	return &SyncManager{config: config}
}

func (s *SyncManager) start() {
	// Initial neighbor list
	s.neighbors.init()

	// Start sync handler
	go s.syncHandler()
}

func (s *SyncManager) syncHandler() {
	// Check if need sync by SyncTickInterval
	ticker := time.NewTicker(SyncTickInterval)
	defer ticker.Stop()

	for range ticker.C {
		// Try to start a syncing progress
		s.StartSyncing()
	}
}

func (s *SyncManager) needSync() (*SPVPeer, bool) {
	// Printout neighbor peers height
	peers := s.getNeighborPeers()
	heights := make([]uint64, 0, len(peers))
	for _, peer := range peers {
		heights = append(heights, peer.Height())
	}
	log.Info("Neighbors -->", heights, s.config.LocalHeight())

	bestPeer := s.getBestPeer()
	if bestPeer == nil { // no peers connected, return false
		log.Info("no peers connected")
		return nil, false
	}
	return bestPeer, bestPeer.Height() > uint64(s.config.LocalHeight())
}

func (s *SyncManager) GetBlocks() {
	s.syncPeer.QueueMessage(s.config.GetBlocks())
}

func (s *SyncManager) AddNeighborPeer(peer *SPVPeer) {
	// Wait for peer quit
	go func() {
		select {
		case <-peer.QuitChan():
			if s.syncPeer != nil && s.syncPeer.ID() == peer.ID() {
				s.syncPeer = nil
			}
			s.delNeighbor(peer.ID())
		}
	}()

	// Set handler's peer
	s.addNeighbor(peer)
}

func (s *SyncManager) StartSyncing() {
	// Check if blockchain need sync
	if bestPeer, needSync := s.needSync(); needSync {
		// Return if already in syncing
		if s.syncPeer != nil {
			return
		}

		// Set sync peer
		s.syncPeer = bestPeer

		// Send getblocks to sync peer
		s.GetBlocks()

	} else {
		// Return if not in syncing
		if s.syncPeer == nil {
			return
		}

		// Clear sync peer
		s.syncPeer = nil

	}
}

func (s *SyncManager) ContinueSync() {
	if s.syncPeer != nil && len(s.syncPeer.blockQueue) == 0 {
		s.GetBlocks()
	}
}

func (s *SyncManager) IsSyncPeer(peer *SPVPeer) bool {
	return s.syncPeer != nil && s.syncPeer.ID() == peer.ID()
}
