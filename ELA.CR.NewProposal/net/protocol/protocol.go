package protocol

import (
	"bytes"
	"encoding/binary"
	"net"
	"time"

	"Elastos.ELA/common"
	"Elastos.ELA/core/ledger"
	"Elastos.ELA/core/transaction"
	. "Elastos.ELA/errors"
	"Elastos.ELA/events"
	"Elastos.ELA/bloom"
)

type NodeAddr struct {
	Time     int64
	Services uint64
	IpAddr   [16]byte
	Port     uint16
	ID       uint64 // Unique ID
}

const (
	MSGCMDLEN          = 12
	CMDOFFSET          = 4
	CHECKSUMLEN        = 4
	HASHLEN            = 32 // hash length in byte
	MSGHDRLEN          = 24
	MAXBLKHDRCNT       = 400
	MAXINVHDRCNT       = 50
	MinConnectionCount = 3
	TIMESOFUPDATETIME  = 2
)

const (
	MAXBUFLEN        = 1024 * 16 // Fixme The maximum buffer to receive message
	PROTOCOLVERSION  = 0
	KEEPALIVETIMEOUT = 3
	DIALTIMEOUT      = 6
	CONNMONITOR      = 6
	MAXSYNCHDRREQ    = 2 //Max Concurrent Sync Header Request
	MaxOutBoundCount = 8
	DefaultMaxPeers  = 125
	MAXIDCACHED      = 5000
)

// The node state
const (
	Init       = 0
	Hand       = 1
	HandShake  = 2
	HandShaked = 3
	Establish  = 4
	Inactive   = 5
)

var ReceiveDuplicateBlockCnt uint64 //an index to detecting networking status

type Noder interface {
	Version() uint32
	GetID() uint64
	Services() uint64
	GetAddr() string
	GetAddr16() ([16]byte, error)
	GetPort() uint16
	GetHttpInfoPort() int
	SetHttpInfoPort(uint16)
	GetState() uint32
	GetRelay() bool
	SetState(state uint32)
	CompareAndSetState(old, new uint32) bool
	LocalNode() Noder
	DelNbrNode(id uint64) (Noder, bool)
	AddNbrNode(Noder)
	CloseConn()
	GetHeight() uint64
	GetConnectionCnt() uint
	GetConn() net.Conn
	GetTxnPool(bool) map[common.Uint256]*transaction.Transaction
	AppendToTxnPool(*transaction.Transaction) ErrCode
	ExistedID(id common.Uint256) bool
	ReqNeighborList()
	DumpInfo()
	UpdateInfo(t time.Time, version uint32, services uint64,
		port uint16, nonce uint64, relay uint8, height uint64)
	ConnectSeeds()
	Connect(nodeAddr string) error
	LoadFilter(filter *bloom.Filter)
	GetFilter() *bloom.Filter
	Tx(buf []byte)
	GetTime() int64
	NodeEstablished(uid uint64) bool
	GetEvent(eventName string) *events.Event
	GetNeighborAddrs() ([]NodeAddr, uint64)
	GetTransaction(hash common.Uint256) *transaction.Transaction
	IncRxTxnCnt()
	GetTxnCnt() uint64
	GetRxTxnCnt() uint64

	Xmit(interface{}) error
	GetNeighborHeights() ([]uint64, uint64)
	WaitForSyncFinish()
	CleanSubmittedTransactions(block *ledger.Block) error
	MaybeAcceptTransaction(txn *transaction.Transaction) error
	RemoveTransaction(txn *transaction.Transaction)

	GetNeighborNoder() []Noder
	GetNbrNodeCnt() uint32
	GetLastRXTime() time.Time
	SetHeight(height uint64)
	IsAddrInNbrList(addr string) bool
	SetAddrInConnectingList(addr string) bool
	RemoveAddrInConnectingList(addr string)
	GetAddressCnt() uint64
	AddAddressToKnownAddress(na NodeAddr)
	RandGetAddresses(nbrAddrs []NodeAddr) []NodeAddr
	NeedMoreAddresses() bool
	RandSelectAddresses() []NodeAddr
	UpdateLastDisconn(id uint64)
	Relay(Noder, interface{}) error
	ExistHash(hash common.Uint256) bool
	IsSyncHeaders() bool
	SetSyncHeaders(b bool)
	IsSyncFailed() bool
	RequestedBlockExisted(hash common.Uint256) bool
	AddRequestedBlock(hash common.Uint256)
	DeleteRequestedBlock(hash common.Uint256)
	GetRequestBlockList() map[common.Uint256]time.Time
	IsNeighborNoder(n Noder) bool
	FindSyncNode() (Noder, error)
	GetBestHeightNoder() Noder
	AcqSyncBlkReqSem()
	RelSyncBlkReqSem()
	AcqSyncHdrReqSem()
	RelSyncHdrReqSem()
	SetStartHash(hash common.Uint256)
	GetStartHash() common.Uint256
	SetStopHash(hash common.Uint256)
	GetStopHash() common.Uint256
	ResetRequestedBlock()
}

func (msg *NodeAddr) Deserialization(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, msg)
	return err
}

func (msg NodeAddr) Serialization() ([]byte, error) {
	var buf bytes.Buffer
	err := binary.Write(&buf, binary.LittleEndian, msg)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), err
}
