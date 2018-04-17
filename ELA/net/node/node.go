package node

import (
	"errors"
	"fmt"
	"net"
	"runtime"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
	"encoding/binary"
	"bytes"
	"crypto/sha256"

	. "Elastos.ELA/common"
	. "Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/ledger"
	"Elastos.ELA/core/transaction"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/net/message"
	. "github.com/elastos/Elastos.ELA/net/protocol"
)

type Semaphore chan struct{}

func MakeSemaphore(n int) Semaphore {
	return make(chan struct{}, n)
}

func (s Semaphore) acquire() { s <- struct{}{} }
func (s Semaphore) release() { <-s }

type node struct {
	//sync.RWMutex	//The Lock not be used as expected to use function channel instead of lock
	state    uint32   // node state
	id       uint64   // The nodes's id
	version  uint32   // The network protocol the node used
	services uint64   // The services the node supplied
	relay    bool     // The relay capability of the node (merge into capbility flag)
	height   uint64   // The node latest block height
	txnCnt   uint64   // The transactions be transmit by this node
	rxTxnCnt uint64   // The transaction received by this node
	// TODO does this channel should be a buffer channel
	chF   chan func() error // Channel used to operate the node without lock
	link                    // The link status and infomation
	local *node             // The pointer to local node
	nbrNodes                // The neighbor node connect with currently node except itself
	eventQueue              // The event queue to notice notice other modules
	TXNPool                 // Unconfirmed transaction pool
	idCache                 // The buffer to store the id of the items which already be processed
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
	log.Info("\t state = ", node.state)
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
		if n.GetState() == Hand || n.GetState() == HandShake || n.GetState() == Establish {
			addr := n.GetAddr()
			port := n.GetPort()
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

	node.Time = t
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

func NewNode() *node {
	n := node{
		state: Init,
		chF:   make(chan func() error),
	}
	runtime.SetFinalizer(&n, rmNode)
	go n.backend()
	return &n
}

func InitNode() Noder {
	n := NewNode()
	n.version = PROTOCOLVERSION

	n.SyncBlkReqSem = MakeSemaphore(MAXSYNCHDRREQ)
	n.SyncHdrReqSem = MakeSemaphore(MAXSYNCHDRREQ)

	n.link.port = uint16(Parameters.NodePort)
	n.relay = true
	idHash := sha256.Sum256([]byte(IPv4Addr() + strconv.Itoa(Parameters.NodePort)))
	binary.Read(bytes.NewBuffer(idHash[:8]), binary.LittleEndian, &(n.id))

	log.Info(fmt.Sprintf("Init node ID to 0x%x", n.id))
	n.nbrNodes.init()
	n.KnownAddressList.init()
	n.local = n
	n.TXNPool.init()
	n.eventQueue.init()
	n.idCache.init()
	n.cachedHashes = make([]Uint256, 0)
	n.nodeDisconnectSubscriber = n.eventQueue.GetEvent("disconnect").Subscribe(events.EventNodeDisconnect, n.NodeDisconnect)
	n.RequestedBlockList = make(map[Uint256]time.Time)
	go n.initConnection()
	go n.updateConnection()
	go n.updateNodeInfo()

	return n
}

func (n *node) NodeDisconnect(v interface{}) {
	if node, ok := v.(*node); ok {
		node.SetState(Inactive)
		conn := node.GetConn()
		conn.Close()
	}
}

func rmNode(node *node) {
	log.Debug(fmt.Sprintf("Remove unused/deuplicate node: 0x%0x", node.id))
}

// TODO pass pointer to method only need modify it
func (node *node) backend() {
	for f := range node.chF {
		f()
	}
}

func (node *node) GetID() uint64 {
	return node.id
}

func (node *node) GetState() uint32 {
	return atomic.LoadUint32(&(node.state))
}

func (node *node) GetConn() net.Conn {
	return node.conn
}

func (node *node) GetPort() uint16 {
	return node.port
}

func (node *node) GetHttpInfoPort() int {
	return int(node.httpInfoPort)
}

func (node *node) SetHttpInfoPort(nodeInfoPort uint16) {
	node.httpInfoPort = nodeInfoPort
}

func (node *node) GetRelay() bool {
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

func (node *node) SetState(state uint32) {
	atomic.StoreUint32(&(node.state), state)
}

func (node *node) CompareAndSetState(old, new uint32) bool {
	return atomic.CompareAndSwapUint32(&(node.state), old, new)
}

func (node *node) LocalNode() Noder {
	return node.local
}

func (node *node) GetHeight() uint64 {
	return node.height
}

func (node *node) SetHeight(height uint64) {
	//TODO read/write lock
	node.height = height
}

func (node *node) Xmit(message interface{}) error {
	log.Debug()
	var buffer []byte
	var err error
	switch message.(type) {
	case *transaction.Transaction:
		log.Debug("TX transaction message")
		txn := message.(*transaction.Transaction)
		buffer, err = NewTxn(txn)
		if err != nil {
			log.Error("Error New Tx message: ", err)
			return err
		}
		node.txnCnt++
	case *ledger.Block:
		log.Debug("TX block message")
		block := message.(*ledger.Block)
		buffer, err = NewBlock(block)
		if err != nil {
			log.Error("Error New Block message: ", err)
			return err
		}
	default:
		log.Warn("Unknown Xmit message type")
		return errors.New("Unknown Xmit message type")
	}

	node.nbrNodes.Broadcast(buffer)

	return nil
}

func (node *node) GetAddr() string {
	return node.addr
}

func (node *node) GetAddr16() ([16]byte, error) {
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
		log.Trace("BlockHeight is ", ledger.DefaultLedger.Blockchain.BlockHeight)
		bc := ledger.DefaultLedger.Blockchain
		log.Info("[", len(bc.Index), len(bc.BlockCache), len(bc.Orphans), "]")

		heights, _ := node.GetNeighborHeights()
		log.Trace("others height is ", heights)

		if CompareHeight(uint64(ledger.DefaultLedger.Blockchain.BlockHeight), heights) {
			node.local.SetSyncHeaders(false)
			break
		}

		<-time.After(5 * time.Second)
	}
}

func (node *node) GetLastRXTime() time.Time {
	return node.Time
}

func (node *node) Relay(frmnode Noder, message interface{}) error {
	log.Debug()
	if node.LocalNode().IsSyncHeaders() == true {
		return nil
	}
	var buffer []byte
	var err error
	isHash := false
	switch message.(type) {
	case *transaction.Transaction:
		log.Debug("TX transaction message")
		txn := message.(*transaction.Transaction)
		buffer, err = NewTxn(txn)
		if err != nil {
			log.Error("Error New Tx message: ", err)
			return err
		}
		node.txnCnt++
	case *ledger.Block:
		log.Debug("TX block message")
		blkpayload := message.(*ledger.Block)
		buffer, err = NewBlock(blkpayload)
		if err != nil {
			log.Error("Error new block message: ", err)
			return err
		}
	default:
		log.Warn("Unknown Relay message type")
		return errors.New("Unknown Relay message type")
	}

	node.nbrNodes.RLock()
	for _, n := range node.nbrNodes.List {
		if n.state == Establish && n.relay == true &&
			n.id != frmnode.GetID() {
			if isHash && n.ExistHash(message.(Uint256)) {
				continue
			}
			n.Tx(buffer)
		}
	}
	node.nbrNodes.RUnlock()
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
	log.Info("nbr heigh-->", heights, ledger.DefaultLedger.Blockchain.BlockHeight)
	if CompareHeight(uint64(ledger.DefaultLedger.Blockchain.BlockHeight), heights) {
		return false
	}
	return true
}

func (node *node) GetBestHeightNoder() Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	var bestnode Noder
	for _, n := range node.nbrNodes.List {
		if n.GetState() == Establish {
			if bestnode == nil {
				if !n.IsSyncFailed() {
					bestnode = n
				}
			} else {
				if (n.GetHeight() > bestnode.GetHeight()) && !n.IsSyncFailed() {
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
