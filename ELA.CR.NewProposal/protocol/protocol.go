package protocol

import (
	"net"
	"time"

	"github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MinConnectionCount = 3
	TimesOfUpdateTime  = 2
)

const (
	ProtocolVersion  = 0
	KeepAliveTimeout = 3
	DialTimeout      = 6
	ConnMonitor      = 6
	MaxSyncHdrReq    = 2 //Max Concurrent Sync Header Request
	MaxOutBoundCount = 8
	DefaultMaxPeers  = 125
	MaxIdCached      = 5000
)

const (
	OpenService = 1 << 2
)

type Noder interface {
	Version() uint32
	ID() uint64
	Services() uint64
	Addr() string
	Addr16() ([16]byte, error)
	Port() uint16
	IsFromExtraNet() bool
	HttpInfoPort() int
	SetHttpInfoPort(uint16)
	SetState(state uint)
	State() uint
	IsRelay() bool
	DelNbrNode(id uint64) (Noder, bool)
	AddNbrNode(Noder)
	Height() uint64
	GetConn() net.Conn
	CloseConn()
	GetConnectionCnt() uint
	GetTransactionPool(bool) map[Uint256]*Transaction
	AppendToTxnPool(*Transaction) ErrCode
	ExistedID(id Uint256) bool
	ReqNeighborList()
	DumpInfo()
	UpdateInfo(t time.Time, version uint32, services uint64,
		port uint16, nonce uint64, relay uint8, height uint64)
	UpdateMsgHelper(handler p2p.MsgHandler)
	ConnectSeeds()
	Connect(nodeAddr string) error
	LoadFilter(filter *msg.FilterLoad)
	BloomFilter() *bloom.Filter
	Send(msg p2p.Message)
	GetTime() int64
	NodeEstablished(uid uint64) bool
	GetEvent(eventName string) *events.Event
	GetNeighborAddrs() ([]p2p.NetAddress, uint64)
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
	AddKnownAddress(na p2p.NetAddress)
	RandGetAddresses(nbrAddrs []p2p.NetAddress) []p2p.NetAddress
	NeedMoreAddresses() bool
	RandSelectAddresses() []p2p.NetAddress
	UpdateLastDisconn(id uint64)
	Relay(Noder, interface{}) error
	ExistHash(hash Uint256) bool
	IsSyncHeaders() bool
	SetSyncHeaders(b bool)
	IsSyncFailed() bool
	IsRequestedBlock(hash Uint256) bool
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
