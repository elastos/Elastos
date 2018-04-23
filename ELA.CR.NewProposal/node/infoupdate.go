package node

import (
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

func (node *node) hasSyncPeer() (bool, Noder) {
	node.local.nbrNodes.RLock()
	defer node.local.nbrNodes.RUnlock()
	noders := node.local.GetNeighborNoder()
	for _, n := range noders {
		if n.IsSyncHeaders() == true {
			return true, n
		}
	}
	return false, nil
}

func (node *node) SyncBlocks() {
	needSync := node.needSync()
	log.Info("needSync: ", needSync)
	if needSync == false {
		node.local.SetSyncHeaders(false)
		syncNode, err := node.FindSyncNode()
		if err == nil {
			syncNode.SetSyncHeaders(false)
			node.local.SetStartHash(EmptyHash)
			node.local.SetStopHash(EmptyHash)
		}
		node.local.ResetRequestedBlock()
	} else {
		hasSyncPeer, syncNode := node.local.hasSyncPeer()
		if hasSyncPeer == false {
			node.LocalNode().ResetRequestedBlock()
			syncNode = node.GetBestHeightNoder()
			hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
			locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

			SendBlocksReq(syncNode, locator, EmptyHash)
		} else {
			list := syncNode.LocalNode().GetRequestBlockList()
			var requests = make(map[Uint256]time.Time, MaxHeaderHashes)
			x := 1
			node.requestedBlockLock.Lock()
			for i, v := range list {
				if x == MaxHeaderHashes {
					break
				}
				requests[i] = v
				x += 1
			}
			node.requestedBlockLock.Unlock()
			if len(requests) == 0 {
				syncNode.SetSyncHeaders(false)
				node.local.SetStartHash(EmptyHash)
				node.local.SetStopHash(EmptyHash)
				syncNode := node.GetBestHeightNoder()
				hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
				locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

				SendBlocksReq(syncNode, locator, EmptyHash)
			} else {
				for hash := range requests {
					if requests[hash].Before(time.Now().Add(-3 * time.Second)) {
						log.Infof("request block hash %x ", hash.Bytes())
						node.LocalNode().AddRequestedBlock(hash)
						go node.Send(msg.NewDataReq(BlockData, hash))
					}
				}
			}
		}
	}
}

func (node *node) SendPingToNbr() {
	noders := node.local.GetNeighborNoder()
	for _, n := range noders {
		if n.State() == ESTABLISH {
			go n.Send(msg.NewPing(chain.DefaultLedger.Store.GetHeight()))
		}
	}
}

func (node *node) HeartBeatMonitor() {
	noders := node.local.GetNeighborNoder()
	periodUpdateTime := config.DEFAULTGENBLOCKTIME / TIMESOFUPDATETIME
	for _, n := range noders {
		if n.State() == ESTABLISH {
			t := n.GetLastActiveTime()
			if t.Before(time.Now().Add(-1 * time.Second * time.Duration(periodUpdateTime) * KEEPALIVETIMEOUT)) {
				log.Warn("keepalive timeout!!!")
				n.SetState(INACTIVITY)
				n.CloseConn()
			}
		}
	}
}

func (node *node) ReqNeighborList() {
	go node.Send(new(msg.AddrsReq))
}

func (node *node) ConnectSeeds() {
	if node.nbrNodes.GetConnectionCnt() < MinConnectionCount {
		seedNodes := config.Parameters.SeedList
		for _, nodeAddr := range seedNodes {
			found := false
			var n Noder
			node.nbrNodes.Lock()
			for _, tn := range node.nbrNodes.List {
				addr := getNodeAddr(tn)
				if nodeAddr == addr.String() {
					n = tn
					found = true
					break
				}
			}
			node.nbrNodes.Unlock()
			if found {
				if n.State() == ESTABLISH {
					if node.LocalNode().NeedMoreAddresses() {
						n.ReqNeighborList()
					}
				}
			} else { //not found
				go node.Connect(nodeAddr)
			}
		}
	}
}

func (node *node) ConnectNode() {
	cntcount := node.nbrNodes.GetConnectionCnt()
	if cntcount < MaxOutBoundCount {
		nbrAddr, _ := node.GetNeighborAddrs()
		addrs := node.RandGetAddresses(nbrAddr)
		for _, addr := range addrs {
			go node.Connect(addr.String())
		}
	}
}

func getNodeAddr(n *node) msg.Addr {
	var addr msg.Addr
	addr.IP, _ = n.Addr16()
	addr.Time = n.GetTime()
	addr.Services = n.Services()
	addr.Port = n.Port()
	addr.ID = n.ID()
	return addr
}

// FIXME part of node info update function could be a node method itself intead of
// a node map method
// Fixme the Nodes should be a parameter
func (node *node) updateNodeInfo() {
	periodUpdateTime := config.DEFAULTGENBLOCKTIME / TIMESOFUPDATETIME
	ticker := time.NewTicker(time.Second * (time.Duration(periodUpdateTime)) * 2)
	for {
		select {
		case <-ticker.C:
			node.SendPingToNbr()
			node.SyncBlocks()
			node.HeartBeatMonitor()
		}
	}
	// TODO when to close the timer
	//close(quit)
}

func (node *node) CheckConnCnt() {
	//compare if connect count is larger than DefaultMaxPeers, disconnect one of the connection
	if node.nbrNodes.GetConnectionCnt() > DefaultMaxPeers {
		disconnNode := node.RandGetANbr()
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, disconnNode)
	}
}

func (node *node) updateConnection() {
	t := time.NewTicker(time.Second * CONNMONITOR)
	for {
		select {
		case <-t.C:
			node.ConnectSeeds()
			node.ConnectNode()
			node.CheckConnCnt()
		}
	}
}
