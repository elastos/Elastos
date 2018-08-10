package node

import (
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/events"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg/v0"
)

type syncTimer struct {
	timeout    time.Duration
	lastUpdate time.Time
	quit       chan struct{}
	onTimeout  func()
}

func newSyncTimer(onTimeout func()) *syncTimer {
	return &syncTimer{
		timeout:   time.Second * SyncBlockTimeout,
		onTimeout: onTimeout,
	}
}

func (t *syncTimer) start() {
	go func() {
		t.quit = make(chan struct{}, 1)
		ticker := time.NewTicker(time.Millisecond * 25)
		defer ticker.Stop()
		for {
			select {
			case <-ticker.C:
				if time.Now().After(t.lastUpdate.Add(t.timeout)) {
					t.onTimeout()
					goto QUIT
				}
			case <-t.quit:
				goto QUIT
			}
		}
	QUIT:
		t.quit = nil
	}()
}

func (t *syncTimer) update() {
	t.lastUpdate = time.Now()
}

func (t *syncTimer) stop() {
	if t.quit != nil {
		t.quit <- struct{}{}
	}
}

func (node *node) hasSyncPeer() (bool, Noder) {
	LocalNode.neighbourNodes.RLock()
	defer LocalNode.neighbourNodes.RUnlock()
	noders := LocalNode.GetNeighborNoder()
	for _, n := range noders {
		if n.IsSyncHeaders() {
			return true, n
		}
	}
	return false, nil
}

func (node *node) SyncBlocks() {
	needSync := node.needSync()
	log.Info("needSync: ", needSync)
	log.Trace("BlockHeight = ", chain.DefaultLedger.Blockchain.BlockHeight)
	chain.DefaultLedger.Blockchain.DumpState()
	bc := chain.DefaultLedger.Blockchain
	log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")
	if needSync {
		hasSyncPeer, syncNode := LocalNode.hasSyncPeer()
		if hasSyncPeer == false {
			LocalNode.ResetRequestedBlock()
			syncNode = node.GetBestHeightNoder()
			hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
			locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

			SendGetBlocks(syncNode, locator, EmptyHash)
			LocalNode.SetSyncHeaders(true)
			syncNode.SetSyncHeaders(true)
			// Start sync timer
			LocalNode.syncTimer.start()
		} else if syncNode.Version() < p2p.EIP001Version {
			list := LocalNode.GetRequestBlockList()
			var requests = make(map[Uint256]time.Time)
			node.requestedBlockLock.Lock()
			for i, v := range list {
				requests[i] = v
				if len(requests) >= p2p.MaxHeaderHashes {
					break
				}
			}
			node.requestedBlockLock.Unlock()
			if len(requests) == 0 {
				syncNode.SetSyncHeaders(false)
				LocalNode.SetStartHash(EmptyHash)
				LocalNode.SetStopHash(EmptyHash)
				syncNode := node.GetBestHeightNoder()
				hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
				locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

				SendGetBlocks(syncNode, locator, EmptyHash)
			} else {
				for hash, t := range requests {
					if time.Now().After(t.Add(time.Second * 3)) {
						log.Infof("request block hash %x ", hash.Bytes())
						LocalNode.AddRequestedBlock(hash)
						syncNode.Send(v0.NewGetData(hash))
					}
				}
			}
		}
	} else {
		LocalNode.stopSyncing()
	}
}

func (node *node) stopSyncing() {
	// Stop sync timer
	LocalNode.syncTimer.stop()
	LocalNode.SetSyncHeaders(false)
	LocalNode.SetStartHash(EmptyHash)
	LocalNode.SetStopHash(EmptyHash)
	syncNode, err := node.FindSyncNode()
	if err == nil {
		syncNode.SetSyncHeaders(false)
	}
}

func (node *node) Heartbeat() {
	ticker := time.NewTicker(time.Second * HeartbeatDuration)
	defer ticker.Stop()
	for range ticker.C {
		// quit when node disconnected
		if !LocalNode.IsNeighborNoder(node) {
			goto QUIT
		}

		// quit when node keep alive timeout
		if time.Now().After(node.lastActive.Add(time.Second * KeepAliveTimeout)) {
			log.Warn("keepalive timeout!!!")
			node.SetState(p2p.INACTIVITY)
			node.CloseConn()
			goto QUIT
		}

		// send ping message to node
		go node.Send(msg.NewPing(chain.DefaultLedger.Store.GetHeight()))
	}
QUIT:
}

func (node *node) RequireNeighbourList() {
	// Do not request addresses from extra node
	if node.IsFromExtraNet() {
		return
	}
	go node.Send(new(msg.GetAddr))
}

func (node *node) ConnectNodes() {
	connectionCount := node.neighbourNodes.GetConnectionCount()
	if connectionCount < MinConnectionCount {
		for _, seed := range config.Parameters.SeedList {
			// Resolve seed address first
			addr, err := resolveAddr(seed)
			if err != nil {
				continue
			}
			log.Debugf("Seed %s, resolved addr %s", seed, addr)
			go node.Connect(addr)
		}
	}

	if connectionCount < MaxOutBoundCount {
		address := node.RandGetAddresses(node.GetNeighbourAddresses())
		for _, addr := range address {
			go node.Connect(addr.String())
		}
	}
	
	if node.NeedMoreAddresses() {
		for _, nbr := range node.GetNeighborNoder() {
			nbr.RequireNeighbourList()
		}
	}

	if connectionCount > DefaultMaxPeers {
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, node.GetANeighbourRandomly())
	}
}

func (node *node) NetAddress() p2p.NetAddress {
	var addr p2p.NetAddress
	addr.IP, _ = node.Addr16()
	addr.Time = node.GetTime()
	addr.Services = node.Services()
	addr.Port = node.Port()
	addr.ID = node.ID()
	return addr
}
