package protocol

import (
	"net"
	"time"

	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MinConnectionCount = 3
	TimesOfUpdateTime  = 2
)

const (
	ProtocolVersion   = 0
	HandshakeTimeout  = 2
	KeepAliveTimeout  = 30
	DialTimeout       = 6
	ConnectionMonitor = 6
	MaxSyncHdrReq     = 2 //Max Concurrent Sync Header Request
	MaxOutBoundCount  = 8
	DefaultMaxPeers   = 125
	MaxIdCached       = 5000
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
	NetAddress() p2p.NetAddress
	Port() uint16
	IsFromExtraNet() bool
	HttpInfoPort() int
	SetHttpInfoPort(uint16)
	SetState(state uint)
	State() uint
	IsRelay() bool
	AddNeighborNode(Noder)
	DelNeighborNode(id uint64) (Noder, bool)
	Height() uint64
	GetConn() net.Conn
	CloseConn()
	AddToHandshakeQueue(node Noder)
	RemoveFromHandshakeQueue(node Noder)
	GetConnectionCount() uint
	GetTransactionPool(bool) map[common.Uint256]*core.Transaction
	AppendToTxnPool(*core.Transaction) errors.ErrCode
	IsDuplicateSidechainTx(sidechainTxHash common.Uint256) bool
	ExistedID(id common.Uint256) bool
	RequireNeighbourList()
	DumpInfo()
	UpdateInfo(t time.Time, version uint32, services uint64,
		port uint16, nonce uint64, relay uint8, height uint64)
	UpdateMsgHelper(handler p2p.MsgHandler)
	ConnectNodes()
	Connect(nodeAddr string) error
	LoadFilter(filter *msg.FilterLoad)
	BloomFilter() *bloom.Filter
	Send(msg p2p.Message)
	GetTime() int64
	NodeEstablished(uid uint64) bool
	GetEvent(eventName string) *events.Event
	GetNeighbourAddresses() []p2p.NetAddress
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
	GetNeighbourCount() uint32
	UpdateLastActive()
	GetLastActiveTime() time.Time
	SetHeight(height uint64)
	IsAddrInNbrList(addr string) bool
	AddToConnectingList(addr string) bool
	RemoveFromConnectingList(addr string)
	GetAddressCnt() uint64
	AddKnownAddress(na p2p.NetAddress)
	RandGetAddresses(nbrAddrs []p2p.NetAddress) []p2p.NetAddress
	NeedMoreAddresses() bool
	RandSelectAddresses() []p2p.NetAddress
	UpdateLastDisconn(id uint64)
	Relay(Noder, interface{}) error
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
