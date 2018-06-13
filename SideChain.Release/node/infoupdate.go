package node

import (
	"time"

	chain "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

func (node *node) hasSyncPeer() (bool, Noder) {
	LocalNode.nbrNodes.RLock()
	defer LocalNode.nbrNodes.RUnlock()
	noders := LocalNode.GetNeighborNoder()
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
	} else {
		LocalNode.ResetRequestedBlock()
		hasSyncPeer, syncNode := LocalNode.hasSyncPeer()
		if hasSyncPeer == false {
			syncNode = node.GetBestHeightNoder()
		}
		hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
		locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

		SendGetBlocks(syncNode, locator, EmptyHash)
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
	periodUpdateTime := config.DEFAULTGENBLOCKTIME / TIMESOFUPDATETIME
	for _, n := range noders {
		if n.State() == p2p.ESTABLISH {
			t := n.GetLastActiveTime()
			if t.Before(time.Now().Add(-1 * time.Second * time.Duration(periodUpdateTime) * KEEPALIVETIMEOUT)) {
				log.Warn("keepalive timeout!!!")
				n.SetState(p2p.INACTIVITY)
				n.CloseConn()
			}
		}
	}
}

func (node *node) ReqNeighborList() {
	go node.Send(new(msg.GetAddr))
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
				if n.State() == p2p.ESTABLISH {
					if LocalNode.NeedMoreAddresses() {
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
		addrs := node.RandGetAddresses(node.GetNeighborAddrs())
		for _, addr := range addrs {
			go node.Connect(addr.String())
		}
	}
}

func getNodeAddr(n *node) p2p.NetAddress {
	var addr p2p.NetAddress
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
