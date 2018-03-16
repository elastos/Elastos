package wallet

import (
	"sync"

	. "SPVWallet/core"
)

const MaxMemCacheSize = 500

type TxnMemCache struct {
	sync.Mutex
	lastTxn  Uint256
	index    int
	txnArray []Uint256
	txnMap   map[Uint256]int
}

func NewTxnMemCache() *TxnMemCache {
	return &TxnMemCache{
		txnArray: make([]Uint256, MaxMemCacheSize),
		txnMap:   make(map[Uint256]int, MaxMemCacheSize),
	}
}

func (cache *TxnMemCache) addToCache(txnId Uint256) {
	// Remove most early cache
	oldTxn := cache.txnArray[cache.index]
	delete(cache.txnMap, oldTxn)
	// Add new txn to it
	cache.txnArray[cache.index] = txnId
	cache.txnMap[txnId] = cache.index
	// Increase index
	cache.lastTxn = txnId
	cache.index = cache.index + 1%MaxMemCacheSize
}

func (cache *TxnMemCache) Cached(txnId Uint256) bool {
	cache.Lock()
	defer cache.Unlock()

	if cache.lastTxn == txnId {
		return true
	}

	if _, ok := cache.txnMap[txnId]; ok {
		return true
	}

	cache.addToCache(txnId)

	return false
}
