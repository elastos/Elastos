package node

import (
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
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
		timeout:   syncBlockTimeout,
		onTimeout: onTimeout,
	}
}

func (t *syncTimer) start() {
	go func() {
		t.quit = make(chan struct{}, 1)
		ticker := time.NewTicker(time.Second)
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

func (node *node) SyncBlocks() {
	needSync := node.needSync()
	log.Info("needSync: ", needSync)
	log.Info("BlockHeight = ", chain.DefaultLedger.Blockchain.BlockHeight)
	chain.DefaultLedger.Blockchain.DumpState()
	bc := chain.DefaultLedger.Blockchain
	log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")
	if needSync {
		syncNode := LocalNode.GetSyncNode()
		if syncNode == nil {
			LocalNode.ResetRequestedBlock()
			syncNode = node.GetBestNode()
			if syncNode == nil {
				return
			}
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
				syncNode := node.GetBestNode()
				if syncNode == nil {
					return
				}
				hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
				locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

				SendGetBlocks(syncNode, locator, EmptyHash)
			} else {
				for hash, t := range requests {
					if time.Now().After(t.Add(syncBlockTimeout)) {
						log.Infof("request block hash %x ", hash.Bytes())
						LocalNode.AddRequestedBlock(hash)
						syncNode.SendMessage(v0.NewGetData(hash))
					}
				}
			}
		}
	} else {
		stopSyncing()
	}
}

func stopSyncing() {
	// Stop sync timer
	LocalNode.syncTimer.stop()
	LocalNode.SetSyncHeaders(false)
	LocalNode.SetStartHash(EmptyHash)
	LocalNode.SetStopHash(EmptyHash)
	syncNode := LocalNode.GetSyncNode()
	if syncNode != nil {
		syncNode.SetSyncHeaders(false)
	}
}

func (node *node) pingHandler() {
	pingTicker := time.NewTicker(pingInterval)
	defer pingTicker.Stop()

out:
	for {
		select {
		case <-pingTicker.C:

			// send ping message to node
			log.Debug("new ping begin")
			node.SendMessage(msg.NewPing(uint64(chain.DefaultLedger.Store.GetHeight())))
			log.Debug("new ping end")

		case <-node.quit:
			break out
		}
	}
}

func (node *node) RequireNeighbourList() {
	// Do not request addresses from external node
	if node.IsExternal() {
		return
	}

	node.SendMessage(&msg.GetAddr{})
}

func (node *node) ConnectNodes() {
	log.Debug()
	internal, total := node.GetConnectionCount()
	if internal < MinConnectionCount {
		for _, seed := range config.Parameters.SeedList {
			node.Connect(seed)
		}
	}

	if total < MaxOutBoundCount {
		for _, addr := range node.RandGetAddresses() {
			node.Connect(addr.String())
		}
	}

	if node.NeedMoreAddresses() {
		for _, nbr := range node.GetNeighborNodes() {
			nbr.RequireNeighbourList()
		}
	}

	if total > DefaultMaxPeers {
		DisconnectNode(node.GetExternalNeighbourRandomly().ID())
	}
}

func (node *node) NetAddress() *p2p.NetAddress {
	return &p2p.NetAddress{
		IP:        node.IP().To4(),
		Timestamp: time.Now(),
		Services:  node.Services(),
		Port:      node.Port(),
	}
}
