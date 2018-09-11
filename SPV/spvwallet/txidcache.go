package spvwallet

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"sync"
)

type TxIdCache struct {
	sync.Mutex
	txIds     map[common.Uint256]struct{}
	index     uint32
	txIdIndex []common.Uint256
}

func NewTxIdCache(capacity int) *TxIdCache {
	return &TxIdCache{
		txIds:     make(map[common.Uint256]struct{}),
		txIdIndex: make([]common.Uint256, capacity),
	}
}

func (ic *TxIdCache) Add(txId common.Uint256) bool {
	ic.Lock()
	defer ic.Unlock()

	// Remove oldest txId
	ic.index = ic.index % uint32(cap(ic.txIdIndex))
	delete(ic.txIds, ic.txIdIndex[ic.index])

	// Add new txId
	ic.txIds[txId] = struct{}{}
	ic.txIdIndex[ic.index] = txId

	// Increase index
	ic.index++
	return true
}

func (ic *TxIdCache) Get(txId common.Uint256) bool {
	ic.Lock()
	defer ic.Unlock()
	_, ok := ic.txIds[txId]
	return ok
}
