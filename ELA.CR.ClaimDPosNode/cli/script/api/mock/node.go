package mock

import (
	"fmt"
	"net"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type NodeMock interface {
	protocol.Noder

	GetTxPool() *mempool.TxPool
	GetBlockPool() *mempool.BlockPool

	GetLastRelay() interface{}
	DumpRelays(level uint32) string
}

func NewNodeMock() NodeMock {
	n := &nodeMock{relayList: make([]interface{}, 0)}
	n.BlockPool.Init()
	n.TxPool.Init()
	return n
}

type nodeMock struct {
	mempool.TxPool
	mempool.BlockPool

	relayList []interface{}
}

func (n *nodeMock) DumpRelays(level uint32) string {
	result := ""

	switch level {
	case 0:
		for _, v := range n.relayList {
			if tx, ok := v.(*types.Transaction); ok {
				result += fmt.Sprintln("[transaction]: type=" + strconv.FormatUint(uint64(tx.TxType), 0))
			} else if blockConfirm, ok := v.(*types.BlockConfirm); ok {
				result += fmt.Sprintln("[block confirm]: HasBlock=" + strconv.FormatBool(blockConfirm.BlockFlag) + " HasConfirm=" + strconv.FormatBool(blockConfirm.ConfirmFlag))
			}
		}
	}

	return result
}

func (n *nodeMock) GetLastRelay() interface{} {
	if len(n.relayList) == 0 {
		return nil
	}
	return n.relayList[len(n.relayList)-1]
}

func (n * nodeMock) GetLastActive() time.Time {
	return time.Time{}
}

func (n *nodeMock) SetLastActive(t time.Time) {

}

func (n *nodeMock) GetTxPool() *mempool.TxPool {
	return &n.TxPool
}

func (n *nodeMock) GetBlockPool() *mempool.BlockPool {
	return &n.BlockPool
}

func (n *nodeMock) Version() uint32 {
	panic("implement me")
}

func (n *nodeMock) ID() uint64 {
	panic("implement me")
}

func (n *nodeMock) Services() uint64 {
	panic("implement me")
}

func (n *nodeMock) Addr() string {
	panic("implement me")
}

func (n *nodeMock) IP() net.IP {
	panic("implement me")
}

func (n *nodeMock) NetAddress() *p2p.NetAddress {
	panic("implement me")
}

func (n *nodeMock) Port() uint16 {
	panic("implement me")
}

func (n *nodeMock) IsExternal() bool {
	panic("implement me")
}

func (n *nodeMock) HttpInfoPort() int {
	panic("implement me")
}

func (n *nodeMock) SetHttpInfoPort(uint16) {
	panic("implement me")
}

func (n *nodeMock) SetState(state protocol.State) {
	panic("implement me")
}

func (n *nodeMock) State() protocol.State {
	panic("implement me")
}

func (n *nodeMock) IsRelay() bool {
	panic("implement me")
}

func (n *nodeMock) IsCurrent() bool {
	panic("implement me")
}

func (n *nodeMock) AddNeighborNode(protocol.Noder) {
	panic("implement me")
}

func (n *nodeMock) DelNeighborNode(id uint64) (protocol.Noder, bool) {
	panic("implement me")
}

func (n *nodeMock) Height() uint64 {
	panic("implement me")
}

func (n *nodeMock) GetConn() net.Conn {
	panic("implement me")
}

func (n *nodeMock) Connected() bool {
	panic("implement me")
}

func (n *nodeMock) Disconnect() {
	panic("implement me")
}

func (n *nodeMock) AddToHandshakeQueue(addr string, node protocol.Noder) {
	panic("implement me")
}

func (n *nodeMock) RemoveFromHandshakeQueue(node protocol.Noder) {
	panic("implement me")
}

func (n *nodeMock) GetConnectionCount() (uint, uint) {
	panic("implement me")
}

func (n *nodeMock) GetTransactionPool(bool) map[common.Uint256]*types.Transaction {
	panic("implement me")
}

func (n *nodeMock) AppendToTxnPool(*types.Transaction) errors.ErrCode {
	panic("implement me")
}

func (n *nodeMock) RegisterTxPoolListener(listener protocol.TxnPoolListener) {
	panic("implement me")
}

func (n *nodeMock) UnregisterTxPoolListener(listener protocol.TxnPoolListener) {
	panic("implement me")
}

func (n *nodeMock) IsDuplicateSidechainTx(sidechainTxHash common.Uint256) bool {
	panic("implement me")
}

func (n *nodeMock) ExistedID(id common.Uint256) bool {
	panic("implement me")
}

func (n *nodeMock) RequireNeighbourList() {
	panic("implement me")
}

func (n *nodeMock) UpdateInfo(t time.Time, version uint32, services uint64,
	port uint16, nonce uint64, relay bool, height uint64) {
	panic("implement me")
}

func (n *nodeMock) UpdateHandler(handler protocol.Handler) {
	panic("implement me")
}

func (n *nodeMock) ConnectNodes() {
	panic("implement me")
}

func (n *nodeMock) Connect(nodeAddr string) error {
	panic("implement me")
}

func (n *nodeMock) LoadFilter(filter *msg.FilterLoad) {
	panic("implement me")
}

func (n *nodeMock) BloomFilter() *bloom.Filter {
	panic("implement me")
}

func (n *nodeMock) SendMessage(msg p2p.Message) {
	panic("implement me")
}

func (n *nodeMock) NodeEstablished(uid uint64) bool {
	panic("implement me")
}

func (n *nodeMock) GetTransaction(hash common.Uint256) *types.Transaction {
	panic("implement me")
}

func (n *nodeMock) IncRxTxnCnt() {
	panic("implement me")
}

func (n *nodeMock) GetTxnCnt() uint64 {
	panic("implement me")
}

func (n *nodeMock) GetRxTxnCnt() uint64 {
	panic("implement me")
}

func (n *nodeMock) GetNeighborNodes() []protocol.Noder {
	panic("implement me")
}

func (n *nodeMock) GetNeighbourAddresses() []*p2p.NetAddress {
	panic("implement me")
}

func (n *nodeMock) WaitForSyncFinish() {
	panic("implement me")
}

func (n *nodeMock) CleanSubmittedTransactions(block *types.Block) error {
	panic("implement me")
}

func (n *nodeMock) MaybeAcceptTransaction(txn *types.Transaction) error {
	panic("implement me")
}

func (n *nodeMock) RemoveTransaction(txn *types.Transaction) {
	panic("implement me")
}

func (n *nodeMock) SetHeight(height uint64) {
	panic("implement me")
}

func (n *nodeMock) Relay(node protocol.Noder, obj interface{}) error {
	n.relayList = append(n.relayList, obj)
	return nil
}

func (n *nodeMock) IsSyncHeaders() bool {
	panic("implement me")
}

func (n *nodeMock) SetSyncHeaders(b bool) {
	panic("implement me")
}

func (n *nodeMock) IsRequestedBlock(hash common.Uint256) bool {
	panic("implement me")
}

func (n *nodeMock) AddRequestedBlock(hash common.Uint256) {
	panic("implement me")
}

func (n *nodeMock) DeleteRequestedBlock(hash common.Uint256) {
	panic("implement me")
}

func (n *nodeMock) GetRequestBlockList() map[common.Uint256]time.Time {
	panic("implement me")
}

func (n *nodeMock) AcqSyncBlkReqSem() {
	panic("implement me")
}

func (n *nodeMock) RelSyncBlkReqSem() {
	panic("implement me")
}

func (n *nodeMock) SetStartHash(hash common.Uint256) {
	panic("implement me")
}

func (n *nodeMock) GetStartHash() common.Uint256 {
	panic("implement me")
}

func (n *nodeMock) SetStopHash(hash common.Uint256) {
	panic("implement me")
}

func (n *nodeMock) GetStopHash() common.Uint256 {
	panic("implement me")
}

func (n *nodeMock) ResetRequestedBlock() {
	panic("implement me")
}
