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
	if needSync == false {
		LocalNode.SetSyncHeaders(false)
		syncNode, err := node.FindSyncNode()
		if err == nil {
			syncNode.SetSyncHeaders(false)
			LocalNode.SetStartHash(EmptyHash)
			LocalNode.SetStopHash(EmptyHash)
		}
		LocalNode.ResetRequestedBlock()
	} else {
		hasSyncPeer, syncNode := LocalNode.hasSyncPeer()
		if hasSyncPeer == false {
			LocalNode.ResetRequestedBlock()
			syncNode = node.GetBestHeightNoder()
			hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
			locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

			SendGetBlocks(syncNode, locator, EmptyHash)
		} else if syncNode.Version() < p2p.EIP001Version {
			list := LocalNode.GetRequestBlockList()
			var requests = make(map[Uint256]time.Time, p2p.MaxHeaderHashes)
			x := 1
			node.requestedBlockLock.Lock()
			for i, v := range list {
				if x == p2p.MaxHeaderHashes {
					break
				}
				requests[i] = v
				x += 1
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
				for hash := range requests {
					if requests[hash].Before(time.Now().Add(-3 * time.Second)) {
						log.Infof("request block hash %x ", hash.Bytes())
						LocalNode.AddRequestedBlock(hash)
						syncNode.Send(v0.NewGetData(hash))
					}
				}
			}
		}
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
		for _, seedAddress := range config.Parameters.SeedList {
			neighbour, ok := existedInNeighbourList(seedAddress, node.neighbourNodes)
			if ok && neighbour.State() == p2p.ESTABLISH {
				neighbour.RequireNeighbourList()
			} else { //not found
				go node.Connect(seedAddress)
			}
		}
	}

	if connectionCount < MaxOutBoundCount {
		address := node.RandGetAddresses(node.GetNeighbourAddresses())
		for _, addr := range address {
			go node.Connect(addr.String())
		}
	}

	if connectionCount > DefaultMaxPeers {
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, node.GetANeighbourRandomly())
	}
}

func existedInNeighbourList(seedAddress string, neighbours neighbourNodes) (Noder, bool) {
	neighbours.Lock()
	defer neighbours.Unlock()

	for _, neighbour := range neighbours.List {
		if seedAddress == neighbour.NetAddress().String() {
			return neighbour, true
		}
	}
	return nil, false
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
