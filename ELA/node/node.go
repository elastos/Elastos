package node

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"encoding/binary"
	"fmt"
	"net"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"time"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	. "github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/bloom"
	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type Semaphore chan struct{}

func MakeSemaphore(n int) Semaphore {
	return make(chan struct{}, n)
}

func (s Semaphore) acquire() { s <- struct{}{} }
func (s Semaphore) release() { <-s }

type node struct {
	//sync.RWMutex	//The Lock not be used as expected to use function channel instead of lock
	PeerState              // node state
	id       uint64        // The nodes's id
	version  uint32        // The network protocol the node used
	services uint64        // The services the node supplied
	relay    bool          // The relay capability of the node (merge into capbility flag)
	height   uint64        // The node latest block height
	txnCnt   uint64        // The transactions be transmit by this node
	rxTxnCnt uint64        // The transaction received by this node
	link                   // The link status and infomation
	local    *node         // The pointer to local node
	nbrNodes               // The neighbor node connect with currently node except itself
	eventQueue             // The event queue to notice notice other modules
	chain.TxPool           // Unconfirmed transaction pool
	idCache                // The buffer to store the id of the items which already be processed
	filter   *bloom.Filter // The bloom filter of a spv node
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
	DefaultMaxPeers          uint
	headerFirstMode          bool
	RequestedBlockList       map[Uint256]time.Time
	SyncBlkReqSem            Semaphore
	SyncHdrReqSem            Semaphore
	StartHash                Uint256
	StopHash                 Uint256
}

type ConnectingNodes struct {
	sync.RWMutex
	ConnectingAddrs []string
}

func (node *node) DumpInfo() {
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
	log.Info("\t conn cnt = ", node.link.connCnt)
}

func (node *node) IsAddrInNbrList(addr string) bool {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	for _, n := range node.nbrNodes.List {
		if n.State() == HAND || n.State() == HANDSHAKE || n.State() == ESTABLISH {
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

func (node *node) SetAddrInConnectingList(addr string) (added bool) {
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

func (node *node) RemoveAddrInConnectingList(addr string) {
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

func (node *node) UpdateInfo(t time.Time, version uint32, services uint64,
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

func NewNode(conn net.Conn) *node {
	node := new(node)
	node.conn = conn
	node.filter = new(bloom.Filter)
	node.MsgReader = NewMsgReader(conn, &MsgHandler{node: node})
	runtime.SetFinalizer(node, rmNode)
	return node
}

func InitNode() Noder {
	Magic = Parameters.Magic

	n := NewNode(nil)
	n.version = PROTOCOLVERSION

	n.SyncBlkReqSem = MakeSemaphore(MAXSYNCHDRREQ)
	n.SyncHdrReqSem = MakeSemaphore(MAXSYNCHDRREQ)

	n.link.port = uint16(Parameters.NodePort)
	if Parameters.SPVService {
		n.services += SPVService
	}
	n.relay = true
	idHash := sha256.Sum256([]byte(IPv4Addr() + strconv.Itoa(Parameters.NodePort)))
	binary.Read(bytes.NewBuffer(idHash[:8]), binary.LittleEndian, &(n.id))

	log.Info(fmt.Sprintf("Init node ID to 0x%x", n.id))
	n.nbrNodes.init()
	n.KnownAddressList.init()
	n.local = n
	n.TxPool.Init()
	n.eventQueue.init()
	n.idCache.init()
	n.cachedHashes = make([]Uint256, 0)
	n.nodeDisconnectSubscriber = n.GetEvent("disconnect").Subscribe(events.EventNodeDisconnect, n.NodeDisconnect)
	n.RequestedBlockList = make(map[Uint256]time.Time)
	n.initConnection()
	go n.updateConnection()
	go n.updateNodeInfo()

	return n
}

func (n *node) NodeDisconnect(v interface{}) {
	if node, ok := v.(*node); ok {
		node.SetState(INACTIVITY)
		conn := node.GetConn()
		conn.Close()
	}
}

func rmNode(node *node) {
	log.Debug(fmt.Sprintf("Remove unused/deuplicate node: 0x%0x", node.id))
}

func (node *node) ID() uint64 {
	return node.id
}

func (node *node) GetConn() net.Conn {
	return node.conn
}

func (node *node) Port() uint16 {
	return node.port
}

func (node *node) LocalPort() uint16 {
	return node.localPort
}

func (node *node) HttpInfoPort() int {
	return int(node.httpInfoPort)
}

func (node *node) SetHttpInfoPort(nodeInfoPort uint16) {
	node.httpInfoPort = nodeInfoPort
}

func (node *node) IsRelay() bool {
	return node.relay
}

func (node *node) Version() uint32 {
	return node.version
}

func (node *node) Services() uint64 {
	return node.services
}

func (node *node) IncRxTxnCnt() {
	node.rxTxnCnt++
}

func (node *node) GetTxnCnt() uint64 {
	return node.txnCnt
}

func (node *node) GetRxTxnCnt() uint64 {
	return node.rxTxnCnt
}

func (node *node) LocalNode() Noder {
	return node.local
}

func (node *node) Height() uint64 {
	return node.height
}

func (node *node) SetHeight(height uint64) {
	node.height = height
}

func (node *node) Addr() string {
	return node.addr
}

func (node *node) Addr16() ([16]byte, error) {
	var result [16]byte
	ip := net.ParseIP(node.addr).To16()
	if ip == nil {
		log.Error("Parse IP address error\n")
		return result, errors.New("Parse IP address error")
	}

	copy(result[:], ip[:16])
	return result, nil
}

func (node *node) GetTime() int64 {
	t := time.Now()
	return t.UnixNano()
}

func (node *node) WaitForSyncFinish() {
	for {
		log.Trace("BlockHeight is ", chain.DefaultLedger.Blockchain.BlockHeight)
		bc := chain.DefaultLedger.Blockchain
		log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")

		heights, _ := node.GetNeighborHeights()
		log.Trace("others height is ", heights)

		if CompareHeight(uint64(chain.DefaultLedger.Blockchain.BlockHeight), heights) {
			node.local.SetSyncHeaders(false)
			break
		}

		<-time.After(5 * time.Second)
	}
}

func (node *node) LoadFilter(filter *bloom.FilterLoad) {
	node.filter.Reload(filter)
}

func (node *node) BloomFilter() *bloom.Filter {
	return node.filter
}

func (node *node) Relay(from Noder, message interface{}) error {
	log.Debug()
	if from != nil && node.LocalNode().IsSyncHeaders() == true {
		return nil
	}

	for _, nbr := range node.GetNeighborNoder() {
		if from == nil || nbr.ID() != from.ID() {

			switch message.(type) {
			case *Transaction:
				log.Debug("TX transaction message")
				txn := message.(*Transaction)

				if nbr.ExistHash(txn.Hash()) {
					continue
				}

				if nbr.IsRelay() || (nbr.BloomFilter().IsLoaded() && nbr.BloomFilter().MatchTxAndUpdate(txn)) {
					go nbr.Send(msg.NewTx(*txn))
					node.txnCnt++
				}
			case *Block:
				log.Debug("TX block message")
				block := message.(*Block)

				if nbr.ExistHash(block.Hash()) {
					continue
				}

				if nbr.BloomFilter().IsLoaded() {
					go nbr.Send(bloom.NewMerkleBlock(block, nbr.BloomFilter()))

				} else if nbr.IsRelay() {
					go nbr.Send(msg.NewBlock(*block))
				}
			default:
				log.Warn("unknown relay message type")
				return errors.New("unknown relay message type")
			}
		}
	}

	return nil
}

func (node *node) ExistHash(hash Uint256) bool {
	node.cachelock.Lock()
	defer node.cachelock.Unlock()
	for _, v := range node.cachedHashes {
		if v == hash {
			return true
		}
	}
	return false
}

func (node node) IsSyncHeaders() bool {
	node.flagLock.RLock()
	defer node.flagLock.RUnlock()
	if (node.syncFlag & 0x01) == 0x01 {
		return true
	} else {
		return false
	}
}

func (node *node) SetSyncHeaders(b bool) {
	node.flagLock.Lock()
	defer node.flagLock.Unlock()
	if b == true {
		node.syncFlag = node.syncFlag | 0x01
	} else {
		node.syncFlag = node.syncFlag & 0xFE
	}
}

func (node node) IsSyncFailed() bool {
	node.flagLock.RLock()
	defer node.flagLock.RUnlock()
	if (node.syncFlag & 0x02) == 0x02 {
		return true
	} else {
		return false
	}
}

func (node *node) needSync() bool {
	heights, _ := node.GetNeighborHeights()
	log.Info("nbr heigh-->", heights, chain.DefaultLedger.Blockchain.BlockHeight)
	if CompareHeight(uint64(chain.DefaultLedger.Blockchain.BlockHeight), heights) {
		return false
	}
	return true
}

func (node *node) GetBestHeightNoder() Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	var bestnode Noder
	for _, n := range node.nbrNodes.List {
		if n.State() == ESTABLISH {
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

func (node *node) GetRequestBlockList() map[Uint256]time.Time {
	return node.RequestedBlockList
}

func (node *node) RequestedBlockExisted(hash Uint256) bool {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	_, ok := node.RequestedBlockList[hash]
	return ok
}

func (node *node) AddRequestedBlock(hash Uint256) {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	node.RequestedBlockList[hash] = time.Now()
}

func (node *node) ResetRequestedBlock() {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()

	node.RequestedBlockList = make(map[Uint256]time.Time)
}

func (node *node) DeleteRequestedBlock(hash Uint256) {
	node.requestedBlockLock.Lock()
	defer node.requestedBlockLock.Unlock()
	_, ok := node.RequestedBlockList[hash]
	if ok == false {
		return
	}
	delete(node.RequestedBlockList, hash)
}

func (node *node) FindSyncNode() (Noder, error) {
	noders := node.local.GetNeighborNoder()
	for _, n := range noders {
		if n.IsSyncHeaders() == true {
			return n, nil
		}
	}
	return nil, errors.New("Not in sync mode")
}

func (node *node) AcqSyncBlkReqSem() {
	node.SyncBlkReqSem.acquire()
}

func (node *node) RelSyncBlkReqSem() {
	node.SyncBlkReqSem.release()
}
func (node *node) AcqSyncHdrReqSem() {
	node.SyncHdrReqSem.acquire()
}

func (node *node) RelSyncHdrReqSem() {
	node.SyncHdrReqSem.release()
}

func (node *node) SetStartHash(hash Uint256) {
	node.StartHash = hash
}

func (node *node) GetStartHash() Uint256 {
	return node.StartHash
}

func (node *node) SetStopHash(hash Uint256) {
	node.StopHash = hash
}

func (node *node) GetStopHash() Uint256 {
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
