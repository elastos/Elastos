package spvwallet

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

type FinishedReqPool struct {
	sync.Mutex
	requests map[Uint256]*BlockTxsRequest
}

func (pool *FinishedReqPool) Add(request *BlockTxsRequest) {
	pool.Lock()
	defer pool.Unlock()

	// Save previous hash as the key
	previous := request.block.BlockHeader.Previous
	// Save genesis block previous to empty
	if request.block.BlockHeader.Height == 1 {
		previous = Uint256{}
	}
	pool.requests[previous] = request
}

func (pool *FinishedReqPool) Next(current Uint256) (*BlockTxsRequest, bool) {
	pool.Lock()
	defer pool.Unlock()

	if block, ok := pool.requests[current]; ok {
		delete(pool.requests, current)
		return block, ok
	}
	return nil, false
}

func (pool *FinishedReqPool) Clear() {
	pool.Lock()
	defer pool.Unlock()

	for hash := range pool.requests {
		delete(pool.requests, hash)
	}
}

func (pool *FinishedReqPool) Length() int {
	pool.Lock()
	defer pool.Unlock()

	return len(pool.requests)
}
