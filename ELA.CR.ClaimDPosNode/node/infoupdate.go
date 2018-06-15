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

func (node *node) SendPingToNbr() {
	noders := LocalNode.GetNeighborNoder()
	for _, n := range noders {
		if n.State() == p2p.ESTABLISH {
			n.Send(msg.NewPing(chain.DefaultLedger.Store.GetHeight()))
		}
	}
}

func (node *node) HeartBeatMonitor() {
	noders := LocalNode.GetNeighborNoder()
	periodUpdateTime := config.DefaultGenBlockTime / TimesOfUpdateTime
	for _, n := range noders {
		if n.State() == p2p.ESTABLISH {
			t := n.GetLastActiveTime()
			if t.Before(time.Now().Add(-1 * time.Second * time.Duration(periodUpdateTime) * KeepAliveTimeout)) {
				log.Warn("keepalive timeout!!!")
				n.SetState(p2p.INACTIVITY)
				n.CloseConn()
			}
		}
	}
}

func (node *node) RequireNeighbourList() {
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
		address := node.RandGetAddresses(node.GetNeighbourAddress())
		for _, addr := range address {
			go node.Connect(addr.String())
		}
	}

	if connectionCount > DefaultMaxPeers {
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, node.GetANeighbourRandomly())
	}
}

func existedInNeighbourList(seedAddress string, neighbours neighbourNodes) (*node, bool) {
	neighbours.Lock()
	defer neighbours.Unlock()

	for _, neighbour := range neighbours.List {
		neighbourAddress := neighbour.getNodeAddress()
		if seedAddress == neighbourAddress {
			return neighbour, true
		}
	}
	return nil, false
}

func (node *node) getNodeAddress() string {
	var addr p2p.NetAddress
	addr.IP, _ = node.Addr16()
	addr.Time = node.GetTime()
	addr.Services = node.Services()
	addr.Port = node.Port()
	addr.ID = node.ID()
	return addr.String()
}
