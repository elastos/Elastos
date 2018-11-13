package mempool

import (
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
)

type BlockPool struct {
	sync.RWMutex
	blockCnt uint64
	blockMap map[common.Uint256]*core.Block
}

func (pool *BlockPool) Init() {
	pool.Lock()
	defer pool.Unlock()
	pool.blockCnt = 0
	pool.blockMap = make(map[common.Uint256]*core.Block)
}

func (pool *BlockPool) AppendToBlockPool(block *core.Block) ErrCode {

	return Success
}

func (pool *BlockPool) ConfirmBlock() ErrCode {

	return Success
}
