package node

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"time"

	chain "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/bloom"
	. "github.com/elastos/Elastos.ELA.SideChain/config"
	. "github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/protocol"
)

var LocalNode Noder

type Node struct {
	//sync.RWMutex	//The Lock not be used as expected to use function channel instead of lock
	p2p.PeerState               // Node state
	id            uint64        // The nodes's id
	version       uint32        // The network protocol the Node used
	services      uint64        // The services the Node supplied
	relay         bool          // The relay capability of the Node (merge into capbility flag)
	height        uint64        // The Node latest block height
	txnCnt        uint64        // The transactions be transmit by this Node
	rxTxnCnt      uint64        // The transaction received by this Node
	link                        // The link status and infomation
	nbrNodes                    // The neighbor Node connect with currently Node except itself
	eventQueue                  // The event queue to notice notice other modules
	chain.TxPool                // Unconfirmed transaction pool
	idCache                     // The buffer to store the id of the items which already be processed
	filter        *bloom.Filter // The bloom filter of a spv Node
	/*
	 * |--|--|--|--|--|--|isSyncFailed|isSyncHeaders|
	 */
	syncFlag                 uint8
	flagLock                 sync.RWMutex
	cachelock                sync.RWMutex
	requestedBlockLock       sync.RWMutex
	nodeDisconnectSubscriber events.Subscriber
	cachedHashes             []Uint256
	ConnectingNodes
	KnownAddressList
	DefaultMaxPeers    uint
	headerFirstMode    bool
	RequestedBlockList map[Uint256]time.Time
	syncTimer          *syncTimer
	SyncBlkReqSem      Semaphore
	SyncHdrReqSem      Semaphore
	StartHash          Uint256
	StopHash           Uint256
}

type ConnectingNodes struct {
	sync.RWMutex
	ConnectingAddrs []string
}

func NewNode(magic uint32, conn net.Conn) *Node {
	node := new(Node)
	node.conn = conn
	node.filter = bloom.LoadFilter(nil)
	node.MsgHelper = p2p.NewMsgHelper(magic, uint32(Parameters.MaxBlockSize), conn, &MsgHandlerV1{node: node})
	runtime.SetFinalizer(node, RmNode)
	return node
}

func InitLocalNode() Noder {
	LocalNode = NewNode(Parameters.Magic, nil)
	LocalNode.SetVersion(ProtocolVersion)

	LocalNode.SetSyncBlkReqSem(MakeSemaphore(MaxSyncHdrReq))
	LocalNode.SetSyncHdrReqSem(MakeSemaphore(MaxSyncHdrReq))

	LocalNode.SetLinkPort(Parameters.NodePort)
	if Parameters.OpenService {
		LocalNode.SetServices(LocalNode.Services() + protocol.OpenService)
	}
	LocalNode.SetRelay(true)
	idHash := sha256.Sum256([]byte(strconv.Itoa(int(time.Now().UnixNano()))))
	id := LocalNode.ID()
	binary.Read(bytes.NewBuffer(idHash[:8]), binary.LittleEndian, &id)
	log.Info(fmt.Sprintf("Init Node ID to 0x%x", id))
	LocalNode.NbrNodesInit()
	LocalNode.KnownAddressListInit()
	LocalNode.TxPoolInit()
	LocalNode.EventQueueInit()
	LocalNode.IdCacheInit()
	LocalNode.CachedHashesInit()
	LocalNode.NodeDisconnectSubscriberInit()
	LocalNode.RequestedBlockListInit()
	LocalNode.HandshakeQueueInit()
	LocalNode.SyncTimerInit()
	LocalNode.InitConnection()

	return LocalNode
}

func UpdateLocalNode() {
	go LocalNode.UpdateConnection()
	go LocalNode.UpdateNodeInfo()
}

func (node *Node) DumpInfo() {
	log.Info("Node info:")
	log.Info("\t state = ", node.State())
	log.Info(fmt.Sprintf("\t id = 0x%x", node.id))
	log.Info("\t addr = ", node.addr)
	log.Info("\t conn = ", node.conn)
	log.Info("\t version = ", node.version)
	log.Info("\t services = ", node.services)
	log.Info("\t port = ", node.port)
	log.Info("\t relay = ", node.relay)
	log.Info("\t height = ", node.height)
}

func (node *Node) IsAddrInNbrList(addr string) bool {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.HAND || n.State() == p2p.HANDSHAKE || n.State() == p2p.ESTABLISH {
			addr := n.Addr()
			port := n.Port()
			na := addr + ":" + strconv.Itoa(int(port))
			if strings.Compare(na, addr) == 0 {
				return true
			}
		}
	}
	return false
}

func (node *Node) AddToConnectionList(addr string) (added bool) {
	node.ConnectingNodes.Lock()
	defer node.ConnectingNodes.Unlock()
	for _, a := range node.ConnectingAddrs {
		if strings.Compare(a, addr) == 0 {
			return false
		}
	}
	node.ConnectingAddrs = append(node.ConnectingAddrs, addr)
	return true
}

func (node *Node) RemoveFromConnectingList(addr string) {
	node.ConnectingNodes.Lock()
	defer node.ConnectingNodes.Unlock()
	addrs := []string{}
	for i, a := range node.ConnectingAddrs {
		if strings.Compare(a, addr) == 0 {
			addrs = append(node.ConnectingAddrs[:i], node.ConnectingAddrs[i+1:]...)
		}
	}
	node.ConnectingAddrs = addrs
}

func (node *Node) UpdateInfo(t time.Time, version uint32, services uint64,
	port uint16, nonce uint64, relay uint8, height uint64) {

	node.lastActive = t
	node.id = nonce
	node.version = version
	node.services = services
	node.port = port
	if relay == 0 {
		node.relay = false
	} else {
		node.relay = true
	}
	node.height = uint64(height)
}

func (n *Node) NodeDisconnect(v interface{}) {
	if node, ok := v.(Noder); ok {
		node.SetState(p2p.INACTIVITY)
		conn := node.GetConn()
		conn.Close()
		LocalNode.DelNbrNode(n.ID())
	}
}

func RmNode(node *Node) {
	log.Debug(fmt.Sprintf("Remove unused/deuplicate Node: 0x%0x", node.id))
}

func (node *Node) ID() uint64 {
	return node.id
}

func (node *Node) SetID(id uint64) {
	node.id = id
}

func (node *Node) GetConn() net.Conn {
	return node.conn
}

func (node *Node) SetConn(conn net.Conn) {
	node.conn = conn
}

func (node *Node) Port() uint16 {
	return node.port
}

func (node *Node) SetPort(port uint16) {
	node.port = port
}

func (node *Node) LinkPort() uint16 {
	return node.link.port
}

func (node *Node) SetLinkPort(port uint16) {
	node.link.port = port
}

func (node *Node) HttpInfoPort() int {
	return int(node.httpInfoPort)
}

func (node *Node) SetHttpInfoPort(nodeInfoPort uint16) {
	node.httpInfoPort = nodeInfoPort
}

func (node *Node) IsRelay() bool {
	return node.relay
}

func (node *Node) SetRelay(relay bool) {
	node.relay = relay
}

func (node *Node) Version() uint32 {
	return node.version
}

func (node *Node) SetVersion(version uint32) {
	node.version = version
}

func (node *Node) Services() uint64 {
	return node.services
}

func (node *Node) SetServices(services uint64) {
	node.services = services
}

func (node *Node) IncRxTxnCnt() {
	node.rxTxnCnt++
}

func (node *Node) GetTxnCnt() uint64 {
	return node.txnCnt
}

func (node *Node) SetTxnCnt(txnCnt uint64) {
	node.txnCnt = txnCnt
}

func (node *Node) GetRxTxnCnt() uint64 {
	return node.rxTxnCnt
}

func (node *Node) SetRxTxnCnt(rxTxnCnt uint64) {
	node.rxTxnCnt = rxTxnCnt
}

func (node *Node) Height() uint64 {
	return node.height
}

func (node *Node) SetHeight(height uint64) {
	node.height = height
}

func (node *Node) Addr() string {
	return node.addr
}

func (node *Node) SetAddr(addr string) {
	node.addr = addr
}

func (node *Node) Addr16() ([16]byte, error) {
	var result [16]byte
	ip := net.ParseIP(node.addr).To16()
	if ip == nil {
		log.Error("Parse IP address error\n")
		return result, errors.New("Parse IP address error")
	}

	copy(result[:], ip[:16])
	return result, nil
}

func (node *Node) GetTime() int64 {
	t := time.Now()
	return t.UnixNano()
}

func (node *Node) WaitForSyncFinish() {
	for {
		log.Trace("BlockHeight is ", chain.DefaultLedger.Blockchain.BlockHeight)
		bc := chain.DefaultLedger.Blockchain
		log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")

		heights := node.GetNeighborHeights()
		log.Trace("others height is ", heights)

		if CompareHeight(uint64(chain.DefaultLedger.Blockchain.BlockHeight), heights) {
			LocalNode.SetSyncHeaders(false)
			break
		}

		<-time.After(5 * time.Second)
	}
}

func (node *Node) LoadFilter(filter *msg.FilterLoad) {
	node.filter.Reload(filter)
}

func (node *Node) BloomFilter() *bloom.Filter {
	return node.filter
}

func (node *Node) Relay(from Noder, message interface{}) error {
	log.Debug()
	if from != nil && LocalNode.IsSyncHeaders() {
		return nil
	}

	for _, nbr := range node.GetNeighborNoder() {
		if from == nil || nbr.ID() != from.ID() {

			switch message := message.(type) {
			case *Transaction:
				log.Debug("Relay transaction message")

				if nbr.ExistHash(message.Hash()) {
					continue
				}

				if nbr.BloomFilter().IsLoaded() && nbr.BloomFilter().MatchTxAndUpdate(message) {
					inv := msg.NewInventory()
					txId := message.Hash()
					inv.AddInvVect(msg.NewInvVect(msg.InvTypeTx, &txId))
					nbr.Send(inv)
					continue
				}

				if nbr.IsRelay() {
					nbr.Send(msg.NewTx(message))
					node.txnCnt++
				}
			case *Block:
				log.Debug("Relay block message")

				if nbr.ExistHash(message.Hash()) {
					continue
				}

				if nbr.BloomFilter().IsLoaded() {
					inv := msg.NewInventory()
					blockHash := message.Hash()
					inv.AddInvVect(msg.NewInvVect(msg.InvTypeBlock, &blockHash))
					nbr.Send(inv)
					continue
				}

				if nbr.IsRelay() {
					nbr.Send(msg.NewBlock(message))
				}
			default:
				log.Warn("unknown relay message type")
				return errors.New("unknown relay message type")
			}
		}
	}

	return nil
}

func (node *Node) ExistHash(hash Uint256) bool {
	node.cachelock.Lock()
	defer node.cachelock.Unlock()
	for _, v := range node.cachedHashes {
		if v == hash {
			return true
		}
	}
	return false
}

func (node Node) IsSyncHeaders() bool {
	node.flagLock.RLock()
	defer node.flagLock.RUnlock()
	if (node.syncFlag & 0x01) == 0x01 {
		return true
	} else {
		return false
	}
}

func (node *Node) SetSyncHeaders(b bool) {
	node.flagLock.Lock()
	defer node.flagLock.Unlock()
	if b == true {
		node.syncFlag = node.syncFlag | 0x01
	} else {
		node.syncFlag = node.syncFlag & 0xFE
	}
}

func (node Node) IsSyncFailed() bool {
	node.flagLock.RLock()
	defer node.flagLock.RUnlock()
	if (node.syncFlag & 0x02) == 0x02 {
		return true
	} else {
		return false
	}
}

func (node *Node) needSync() bool {
	heights := node.GetNeighborHeights()
	log.Info("nbr heigh-->", heights, chain.DefaultLedger.Blockchain.BlockHeight)
	if CompareHeight(uint64(chain.DefaultLedger.Blockchain.BlockHeight), heights) {
		return false
	}
	return true
}

func (node *Node) GetBestHeightNoder() Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	var bestnode Noder
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.ESTABLISH {
			if bestnode == nil {
				if !n.IsSyncFailed() {
					bestnode = n
				}
			} else {
				if (n.Height() > bestnode.Height()) && !n.IsSyncFailed() {
					bestnode = n
				}
			}
		}
	}
	return bestnode
}

func (node *Node) GetRequestBlockList() map[Uint256]time.Time {
	return node.RequestedBlockList
}

func (node *Node) IsRequestedBlock(hash Uint256) bool {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	_, ok := node.RequestedBlockList[hash]
	return ok
}

func (node *Node) AddRequestedBlock(hash Uint256) {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	node.RequestedBlockList[hash] = time.Now()
}

func (node *Node) ResetRequestedBlock() {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()

	node.RequestedBlockList = make(map[Uint256]time.Time)
}

func (node *Node) DeleteRequestedBlock(hash Uint256) {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	_, ok := node.RequestedBlockList[hash]
	if ok == false {
		return
	}
	delete(node.RequestedBlockList, hash)
}

func (node *Node) FindSyncNode() (Noder, error) {
	noders := LocalNode.GetNeighborNoder()
	for _, n := range noders {
		if n.IsSyncHeaders() == true {
			return n, nil
		}
	}
	return nil, errors.New("Not in sync mode")
}

func (node *Node) GetSyncBlkReqSem() Semaphore {
	return node.SyncBlkReqSem
}

func (node *Node) SetSyncBlkReqSem(syncBlkReqSem Semaphore) {
	node.SyncBlkReqSem = syncBlkReqSem
}

func (node *Node) GetSyncHdrReqSem() Semaphore {
	return node.SyncHdrReqSem
}

func (node *Node) SetSyncHdrReqSem(syncHdrReqSem Semaphore) {
	node.SyncHdrReqSem = syncHdrReqSem
}

func (node *Node) AcqSyncBlkReqSem() {
	node.SyncBlkReqSem.Acquire()
}

func (node *Node) RelSyncBlkReqSem() {
	node.SyncBlkReqSem.Release()
}

func (node *Node) AcqSyncHdrReqSem() {
	node.SyncHdrReqSem.Acquire()
}

func (node *Node) RelSyncHdrReqSem() {
	node.SyncHdrReqSem.Release()
}

func (node *Node) SetStartHash(hash Uint256) {
	node.StartHash = hash
}

func (node *Node) GetStartHash() Uint256 {
	return node.StartHash
}

func (node *Node) SetStopHash(hash Uint256) {
	node.StopHash = hash
}

func (node *Node) GetStopHash() Uint256 {
	return node.StopHash
}

func CompareHeight(blockHeight uint64, heights []uint64) bool {
	for _, height := range heights {
		if blockHeight < height {
			return false
		}
	}
	return true
}

func (node *Node) NbrNodesInit() {
	node.nbrNodes.init()
}

func (node *Node) KnownAddressListInit() {
	node.KnownAddressList.init()
}

func (node *Node) TxPoolInit() {
	node.TxPool.Init()
}

func (node *Node) EventQueueInit() {
	node.eventQueue.init()
}

func (node *Node) IdCacheInit() {
	node.idCache.init()
}

func (node *Node) CachedHashesInit() {
	node.cachedHashes = make([]Uint256, 0)
}

func (node *Node) NodeDisconnectSubscriberInit() {
	node.nodeDisconnectSubscriber = node.GetEvent("disconnect").Subscribe(events.EventNodeDisconnect, node.NodeDisconnect)
}

func (node *Node) RequestedBlockListInit() {
	node.RequestedBlockList = make(map[Uint256]time.Time)
}

func (node *Node) HandshakeQueueInit() {
	node.handshakeQueue.init()
}

func (node *Node) SyncTimerInit() {
	node.syncTimer = newSyncTimer(node.stopSyncing)
}

func (node *Node) UpdateSyncTimer() {
	node.syncTimer.update()
}
