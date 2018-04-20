package protocol

import (
	"net"
	"time"

	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"

	"github.com/elastos/Elastos.ELA.Utility/bloom"
	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MSGHDRLEN          = 24
	MAXBLKHDRCNT       = 400
	MinConnectionCount = 3
	TIMESOFUPDATETIME  = 2
)

const (
	MAXBUFLEN        = 1024 * 16 // Fixme The maximum buffer to receive message
	PROTOCOLVERSION  = 1
	KEEPALIVETIMEOUT = 3
	DIALTIMEOUT      = 6
	CONNMONITOR      = 6
	MAXSYNCHDRREQ    = 2 //Max Concurrent Sync Header Request
	MaxOutBoundCount = 8
	DefaultMaxPeers  = 125
	MAXIDCACHED      = 5000
)

const (
	SPVPort    = 20866
	SPVService = 1 << 2
)

type Noder interface {
	Version() uint32
	ID() uint64
	Services() uint64
	Addr() string
	Addr16() ([16]byte, error)
	Port() uint16
	LocalPort() uint16
	HttpInfoPort() int
	SetHttpInfoPort(uint16)
	SetState(state uint)
	State() uint
	IsRelay() bool
	LocalNode() Noder
	DelNbrNode(id uint64) (Noder, bool)
	AddNbrNode(Noder)
	Height() uint64
	GetConn() net.Conn
	CloseConn()
	GetConnectionCnt() uint
	GetTxnPool(bool) map[Uint256]*Transaction
	AppendToTxnPool(*Transaction) ErrCode
	ExistedID(id Uint256) bool
	ReqNeighborList()
	DumpInfo()
	UpdateInfo(t time.Time, version uint32, services uint64,
		port uint16, nonce uint64, relay uint8, height uint64)
	ConnectSeeds()
	Connect(nodeAddr string) error
	LoadFilter(filter *bloom.FilterLoad)
	BloomFilter() *bloom.Filter
	Send(msg p2p.Message)
	GetTime() int64
	NodeEstablished(uid uint64) bool
	GetEvent(eventName string) *events.Event
	GetNeighborAddrs() ([]msg.Addr, uint64)
	GetTransaction(hash Uint256) *Transaction
	IncRxTxnCnt()
	GetTxnCnt() uint64
	GetRxTxnCnt() uint64

	GetNeighborHeights() ([]uint64, uint64)
	WaitForSyncFinish()
	CleanSubmittedTransactions(block *Block) error
	MaybeAcceptTransaction(txn *Transaction) error
	RemoveTransaction(txn *Transaction)

	GetNeighborNoder() []Noder
	GetNbrNodeCnt() uint32
	UpdateLastActive()
	GetLastActiveTime() time.Time
	SetHeight(height uint64)
	IsAddrInNbrList(addr string) bool
	SetAddrInConnectingList(addr string) bool
	RemoveAddrInConnectingList(addr string)
	GetAddressCnt() uint64
	AddAddressToKnownAddress(na msg.Addr)
	RandGetAddresses(nbrAddrs []msg.Addr) []msg.Addr
	NeedMoreAddresses() bool
	RandSelectAddresses() []msg.Addr
	UpdateLastDisconn(id uint64)
	Relay(Noder, interface{}) error
	ExistHash(hash Uint256) bool
	IsSyncHeaders() bool
	SetSyncHeaders(b bool)
	IsSyncFailed() bool
	RequestedBlockExisted(hash Uint256) bool
	AddRequestedBlock(hash Uint256)
	DeleteRequestedBlock(hash Uint256)
	GetRequestBlockList() map[Uint256]time.Time
	IsNeighborNoder(n Noder) bool
	FindSyncNode() (Noder, error)
	GetBestHeightNoder() Noder
	AcqSyncBlkReqSem()
	RelSyncBlkReqSem()
	AcqSyncHdrReqSem()
	RelSyncHdrReqSem()
	SetStartHash(hash Uint256)
	GetStartHash() Uint256
	SetStopHash(hash Uint256)
	GetStopHash() Uint256
	ResetRequestedBlock()
}
