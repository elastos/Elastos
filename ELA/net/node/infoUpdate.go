package node

import (
	"Elastos.ELA/common"
	"Elastos.ELA/config"
	"Elastos.ELA/log"
	"Elastos.ELA/core/ledger"
	"Elastos.ELA/events"
	. "Elastos.ELA/net/message"
	. "Elastos.ELA/net/protocol"
	"net"
	"strconv"
	"time"
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

func (node *node) SyncBlks() {
	needSync := node.needSync()
	log.Info("needSync: ", needSync)
	if needSync == false {
		node.local.SetSyncHeaders(false)
		syncNode, err := node.FindSyncNode()
		if err == nil {
			syncNode.SetSyncHeaders(false)
			var emptyHash common.Uint256
			node.local.SetStartHash(emptyHash)
			node.local.SetStopHash(emptyHash)
		}
		node.LocalNode().ResetRequestedBlock()
	} else {
		var syncNode Noder
		hasSyncPeer, syncNode := node.local.hasSyncPeer()
		if hasSyncPeer == false {
			node.LocalNode().ResetRequestedBlock()
			syncNode = node.GetBestHeightNoder()
			hash := ledger.DefaultLedger.Store.GetCurrentBlockHash()

			blocator := ledger.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)
			var emptyHash common.Uint256
			SendMsgSyncBlockHeaders(syncNode, blocator, emptyHash)
		} else {
			//rb := syncNode.LocalNode().GetRequestBlockList()
			rb1 := syncNode.LocalNode().GetRequestBlockList()
			var rb = make(map[common.Uint256]time.Time, 50)
			x := 1
			node.requestedBlockLock.Lock()
			for i, v := range rb1 {
				if x == 50 {
					break
				}
				rb[i] = v
				x += 1
			}
			node.requestedBlockLock.Unlock()
			if len(rb) == 0 {
				syncNode.SetSyncHeaders(false)
				var emptyHash common.Uint256
				node.local.SetStartHash(emptyHash)
				node.local.SetStopHash(emptyHash)
				newSyncNode := node.GetBestHeightNoder()
				hash := ledger.DefaultLedger.Store.GetCurrentBlockHash()
				blocator := ledger.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)
				SendMsgSyncBlockHeaders(newSyncNode, blocator, emptyHash)
			} else {
				for k := range rb {
					if rb[k].Before(time.Now().Add(-3 * time.Second)) {
						log.Infof("request block hash %x ", k.Bytes())
						<-time.After(time.Millisecond * 50)
						ReqBlkData(syncNode, k)
					}
				}
			}
		}
	}
}

func (node *node) SendPingToNbr() {
	noders := node.local.GetNeighborNoder()
	for _, n := range noders {
		if n.State() == Establish {
			buf, err := NewPingMsg()
			if err != nil {
				log.Error("failed build a new ping message")
			} else {
				go n.Tx(buf)
			}
		}
	}
}

func (node *node) HeartBeatMonitor() {
	noders := node.local.GetNeighborNoder()
	periodUpdateTime := config.DEFAULTGENBLOCKTIME / TIMESOFUPDATETIME
	for _, n := range noders {
		if n.State() == Establish {
			t := n.GetLastRXTime()
			if t.Before(time.Now().Add(-1 * time.Second * time.Duration(periodUpdateTime) * KEEPALIVETIMEOUT)) {
				log.Warn("keepalive timeout!!!")
				n.SetState(Inactive)
				n.CloseConn()
			}
		}
	}
}

func (node *node) ReqNeighborList() {
	buf, _ := NewMsg("getaddr", node.local)
	go node.Tx(buf)
}

func (node *node) ConnectSeeds() {
	if node.nbrNodes.GetConnectionCnt() < MinConnectionCount {
		seedNodes := config.Parameters.SeedList
		for _, nodeAddr := range seedNodes {
			found := false
			var n Noder
			var ip net.IP
			node.nbrNodes.Lock()
			for _, tn := range node.nbrNodes.List {
				addr := getNodeAddr(tn)
				ip = addr.IpAddr[:]
				addrstring := ip.To16().String() + ":" + strconv.Itoa(int(addr.Port))
				if nodeAddr == addrstring {
					n = tn
					found = true
					break
				}
			}
			node.nbrNodes.Unlock()
			if found {
				if n.State() == Establish {
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
		for _, nodeAddr := range addrs {
			addr := nodeAddr.IpAddr
			port := nodeAddr.Port
			var ip net.IP
			ip = addr[:]
			na := ip.To16().String() + ":" + strconv.Itoa(int(port))
			go node.Connect(na)
		}
	}
}

func getNodeAddr(n *node) NodeAddr {
	var addr NodeAddr
	addr.IpAddr, _ = n.Addr16()
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
			node.SyncBlks()
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
		node.eventQueue.GetEvent("disconnect").Notify(events.EventNodeDisconnect, disconnNode)
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
