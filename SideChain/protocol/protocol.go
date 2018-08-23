package protocol

import (
	"net"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/bloom"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/events"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	ProtocolVersion    = 1
	HandshakeTimeout   = 2
	MinConnectionCount = 3
	KeepAliveTimeout   = 30
	DialTimeout        = 6
	SyncBlockTimeout   = 10
	HeartbeatDuration  = 6
	MaxSyncHdrReq      = 2 //Max Concurrent Sync Header Request
	MaxOutBoundCount   = 8
	DefaultMaxPeers    = 125
	MaxIdCached        = 5000
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
	HttpInfoPort() int
	SetHttpInfoPort(uint16)
	SetState(state uint)
	State() uint
	IsRelay() bool
	Heartbeat()
	DelNbrNode(id uint64) (Noder, bool)
	AddNbrNode(Noder)
	Height() uint64
	GetConn() net.Conn
	CloseConn()
	GetConnectionCnt() uint
	GetTxsInPool() map[common.Uint256]*core.Transaction
	AppendToTxnPool(*core.Transaction) errors.ErrCode
	IsDuplicateMainchainTx(mainchainTxHash common.Uint256) bool
	ExistedID(id common.Uint256) bool
	DumpInfo()
	UpdateInfo(t time.Time, version uint32, services uint64,
		port uint16, nonce uint64, relay uint8, height uint64)
	ConnectSeeds()
	Connect(nodeAddr string) error
	LoadFilter(filter *msg.FilterLoad)
	BloomFilter() *bloom.Filter
	Send(msg p2p.Message)
	GetTime() int64
	NodeEstablished(uid uint64) bool
	GetEvent(eventName string) *events.Event
	GetNeighborAddrs() []p2p.NetAddress
	GetTransaction(hash common.Uint256) *core.Transaction
	IncRxTxnCnt()
	GetTxnCnt() uint64
	GetRxTxnCnt() uint64

	GetNeighborHeights() []uint64
	WaitForSyncFinish()
	CleanSubmittedTransactions(block *core.Block) error
	MaybeAcceptTransaction(txn *core.Transaction) error
	RemoveTransaction(txn *core.Transaction)

	GetNeighborNoder() []Noder
	GetNbrNodeCnt() uint32
	UpdateLastActive()
	GetLastActiveTime() time.Time
	SetHeight(height uint64)
	IsAddrInNbrList(addr string) bool
	GetAddressCnt() uint64
	AddAddressToKnownAddress(na p2p.NetAddress)
	RandGetAddresses(nbrAddrs []p2p.NetAddress) []p2p.NetAddress
	NeedMoreAddresses() bool
	RandSelectAddresses() []p2p.NetAddress
	UpdateLastDisconn(id uint64)
	Relay(Noder, interface{}) error
	ExistHash(hash common.Uint256) bool
	IsSyncHeaders() bool
	SetSyncHeaders(b bool)
	IsSyncFailed() bool
	IsRequestedBlock(hash common.Uint256) bool
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
