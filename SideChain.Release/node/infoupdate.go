package node

import (
	"time"

	chain "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
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
	if needSync {
		if LocalNode.IsSyncHeaders() {
			return
		}
		LocalNode.ResetRequestedBlock()
		hasSyncPeer, syncNode := LocalNode.hasSyncPeer()
		if hasSyncPeer == false {
			syncNode = node.GetBestHeightNoder()
		}
		hash := chain.DefaultLedger.Store.GetCurrentBlockHash()
		locator := chain.DefaultLedger.Blockchain.BlockLocatorFromHash(&hash)

		SendGetBlocks(syncNode, locator, common.EmptyHash)
		LocalNode.SetSyncHeaders(true)
		syncNode.SetSyncHeaders(true)
		// Start sync timer
		LocalNode.syncTimer.start()
	} else {
		LocalNode.stopSyncing()
	}
}

func (node *node) stopSyncing() {
	// Stop sync timer
	LocalNode.syncTimer.stop()
	LocalNode.SetSyncHeaders(false)
	syncNode, err := node.FindSyncNode()
	if err == nil {
		syncNode.SetSyncHeaders(false)
		LocalNode.SetStartHash(common.EmptyHash)
		LocalNode.SetStopHash(common.EmptyHash)
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
						n.Send(new(msg.Addr))
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

func (node *node) updateNodeInfo() {
	ticker := time.NewTicker(time.Second * HeartbeatDuration)
	for {
		select {
		case <-ticker.C:
			node.SyncBlocks()
		}
	}
}

func (node *node) CheckConnCnt() {
	//compare if connect count is larger than DefaultMaxPeers, disconnect one of the connection
	if node.nbrNodes.GetConnectionCnt() > DefaultMaxPeers {
		disconnNode := node.RandGetANbr()
		node.GetEvent("disconnect").Notify(events.EventNodeDisconnect, disconnNode)
	}
}

func (node *node) updateConnection() {
	t := time.NewTicker(time.Second * HeartbeatDuration)
	for {
		select {
		case <-t.C:
			node.ConnectSeeds()
			node.ConnectNode()
			node.CheckConnCnt()
		}
	}
}
