package spvwallet

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
)

type FinishedReqPool struct {
	sync.Mutex
	genesis  *Uint256
	blocks   map[Uint256]*bloom.MerkleBlock
	requests map[Uint256]*BlockTxsRequest
	lastPop  *Uint256
}

func (pool *FinishedReqPool) Add(request *BlockTxsRequest) {
	pool.Lock()
	defer pool.Unlock()

	// Save previous hash as the key
	previous := request.block.BlockHeader.Previous
	// Save genesis block previous to empty
	if request.block.BlockHeader.Height == 1 {
		pool.genesis = &previous
	}
	pool.requests[previous] = request
	// Save finished block
	pool.blocks[request.blockHash] = &request.block

	log.Debug("Finished pool add block: ", previous.String(), ", height: ", request.block.BlockHeader.Height)
}

func (pool *FinishedReqPool) Contain(hash Uint256) (*bloom.MerkleBlock, bool) {
	pool.Lock()
	defer pool.Unlock()

	block, ok := pool.blocks[hash]
	return block, ok
}

func (pool *FinishedReqPool) Next(current Uint256) (*BlockTxsRequest, bool) {
	pool.Lock()
	defer pool.Unlock()

	// Any time return genesis block first
	if pool.genesis != nil {
		current = *pool.genesis
		pool.genesis = nil
	}

	log.Debug("Finished pool get next key: ", current.String())
	if request, ok := pool.requests[current]; ok {
		delete(pool.requests, current)
		delete(pool.blocks, request.blockHash)
		pool.lastPop = &request.blockHash
		return request, ok
	}
	return nil, false
}

func (pool *FinishedReqPool) LastPop() *Uint256 {
	return pool.lastPop
}

func (pool *FinishedReqPool) Clear() {
	pool.Lock()
	defer pool.Unlock()

	for hash := range pool.blocks {
		delete(pool.blocks, hash)
	}
	for hash := range pool.requests {
		delete(pool.requests, hash)
	}
	pool.lastPop = nil
}

func (pool *FinishedReqPool) Length() int {
	pool.Lock()
	defer pool.Unlock()

	return len(pool.requests)
}
