package spv

import (
	"sync"

	. "SPVWallet/core"
	. "SPVWallet/msg"
)

const MaxMemCacheSize = 500

type MemCache struct {
	lock     *sync.Mutex
	lastTxn  Uint256
	index    int
	txnArray []Uint256
	hashMap  map[Uint256]int

	orphanBlocks map[Uint256]*MerkleBlock
	orphanTxns   map[Uint256]*Txn
}

func NewMemCache() *MemCache {
	return &MemCache{
		lock:         new(sync.Mutex),
		txnArray:     make([]Uint256, MaxMemCacheSize),
		hashMap:      make(map[Uint256]int, MaxMemCacheSize),
		orphanBlocks: make(map[Uint256]*MerkleBlock),
		orphanTxns:   make(map[Uint256]*Txn),
	}
}

func (cache *MemCache) addToCache(hash Uint256) {
	// Remove most early cache
	oldHash := cache.txnArray[cache.index]
	delete(cache.hashMap, oldHash)
	delete(cache.orphanBlocks, oldHash)
	delete(cache.orphanTxns, oldHash)
	// Add new txn to it
	cache.txnArray[cache.index] = hash
	cache.hashMap[hash] = cache.index
	// Increase index
	cache.lastTxn = hash
	cache.index = (cache.index + 1) % MaxMemCacheSize
}

func (cache *MemCache) TxCached(txId Uint256) bool {
	cache.lock.Lock()
	defer cache.lock.Unlock()

	if cache.lastTxn == txId {
		return true
	}

	return cache.cached(txId)
}

func (cache *MemCache) cached(hash Uint256) bool {

	if _, ok := cache.hashMap[hash]; ok {
		return true
	}

	cache.addToCache(hash)

	return false
}

func (cache *MemCache) IsOrphanBlock(hash Uint256) (*MerkleBlock, bool) {
	cache.lock.Lock()
	defer cache.lock.Unlock()

	block, ok := cache.orphanBlocks[hash]
	return block, ok
}

func (cache *MemCache) AddOrphanBlock(blockHash Uint256, block *MerkleBlock) {
	cache.lock.Lock()
	defer cache.lock.Unlock()

	cache.orphanBlocks[blockHash] = block
	cache.cached(blockHash)
}

func (cache *MemCache) IsOrphanTxn(hash Uint256) (*Txn, bool) {
	cache.lock.Lock()
	defer cache.lock.Unlock()

	txn, ok := cache.orphanTxns[hash]
	return txn, ok
}

func (cache *MemCache) AddOrphanTxn(txId Uint256, txn *Txn) {
	cache.lock.Lock()
	defer cache.lock.Unlock()

	cache.orphanTxns[txId] = txn
	cache.cached(txId)
}
