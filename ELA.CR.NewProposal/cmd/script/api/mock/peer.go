package mock

import (
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type PeerMock interface {
	GetTxPool() *mempool.TxPool
	GetBlockPool() *mempool.BlockPool

	GetLastRelay() interface{}
	DumpRelays(level uint32) string
	Broadcast(msg p2p.Message)
}

func NewPeerMock(params *config.Params) PeerMock {
	p := &peerMock{
		TxPool:    mempool.NewTxPool(params),
		BlockPool: mempool.NewBlockPool(params),
		relayList: make([]p2p.Message, 0),
	}
	p.BlockPool.Chain = blockchain.DefaultLedger.Blockchain

	return p
}

type peerMock struct {
	*mempool.TxPool
	*mempool.BlockPool

	relayList []p2p.Message
}

func (n *peerMock) DumpRelays(level uint32) string {
	result := ""

	switch level {
	case 0:
		for _, v := range n.relayList {
			if m, ok := v.(*msg.Tx); ok {
				tx := m.Serializable.(*types.Transaction)
				result += fmt.Sprintf("[transaction]: type=%s",
					tx.TxType.Name())
			} else if m, ok := v.(*msg.Block); ok {
				block := m.Serializable.(*types.DposBlock)
				result += fmt.Sprintf("[block confirm]: HasBlock=%t"+
					" HasConfirm=%t", true, block.HaveConfirm)
			}
		}
	}

	return result
}

func (n *peerMock) GetLastRelay() interface{} {
	if len(n.relayList) == 0 {
		return nil
	}
	return n.relayList[len(n.relayList)-1]
}

func (n *peerMock) GetLastActive() time.Time {
	return time.Time{}
}

func (n *peerMock) SetLastActive(t time.Time) {

}

func (n *peerMock) GetTxPool() *mempool.TxPool {
	return n.TxPool
}

func (n *peerMock) GetBlockPool() *mempool.BlockPool {
	return n.BlockPool
}

func (n *peerMock) Broadcast(msg p2p.Message) {
	n.relayList = append(n.relayList, msg)
}
