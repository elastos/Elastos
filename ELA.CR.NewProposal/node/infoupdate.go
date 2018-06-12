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
	LocalNode.nbrNodes.RLock()
	defer LocalNode.nbrNodes.RUnlock()
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

func (node *node) ReqNeighborList() {
	go node.Send(new(msg.GetAddr))
}

func (node *node) ConnectSeeds() {
	if node.nbrNodes.GetConnectionCnt() < MinConnectionCount {
		for _, nodeAddr := range config.Parameters.SeedList {
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
	periodUpdateTime := config.DefaultGenBlockTime / TimesOfUpdateTime
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
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, node.RandGetANbr())
	}
}

func (node *node) updateConnection() {
	t := time.NewTicker(time.Second * ConnMonitor)
	for {
		select {
		case <-t.C:
			node.ConnectSeeds()
			node.ConnectNode()
			node.CheckConnCnt()
		}
	}
}
