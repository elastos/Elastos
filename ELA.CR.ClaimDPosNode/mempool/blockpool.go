package mempool

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
)

type BlockPool struct {
	sync.RWMutex
	blockCnt   uint64
	blockMap   map[common.Uint256]*types.Block
	confirmMap map[common.Uint256]*types.DPosProposalVoteSlot
}

func (pool *BlockPool) Init() {
	pool.Lock()
	defer pool.Unlock()

	pool.blockMap = make(map[common.Uint256]*types.Block)
	pool.confirmMap = make(map[common.Uint256]*types.DPosProposalVoteSlot)
}

func (pool *BlockPool) AppendDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	log.Debugf("[AppendDposBlock] start")
	defer log.Debugf("[AppendDposBlock] end")

	// add block
	block := dposBlock.Block
	hash := block.Hash()
	if _, exist := pool.GetBlock(hash); exist {
		return false, false, errors.New("duplicate block in pool")
	}
	// verify block
	if err := blockchain.PowCheckBlockSanity(block, config.Parameters.ChainParam.PowLimit,
		blockchain.DefaultLedger.Blockchain.TimeSource); err != nil {
		return false, false, err
	}
	pool.AddToBlockMap(block)

	// add confirm
	if dposBlock.ConfirmFlag {
		inMainChain, isOrphan, err := pool.AppendConfirm(dposBlock.Confirm)
		if err != nil {
			return false, false, err
		}
		return inMainChain, isOrphan, nil
	}

	// confirm block
	isConfirmed := true
	inMainChain, isOrphan, err := pool.ConfirmBlock(hash)
	if err != nil {
		log.Debug("[AppendDposBlock] ConfirmBlock failed, hash:", hash.String(), "err: ", err)
		isConfirmed = false
	}

	// notify arbiter new block received
	if blockchain.DefaultLedger.Blockchain.NewBlocksListeners != nil {
		for _, v := range blockchain.DefaultLedger.Blockchain.NewBlocksListeners {
			v.OnBlockReceived(block, isConfirmed)
		}
	}
	return inMainChain, isOrphan, nil
}

func (pool *BlockPool) AppendConfirm(confirm *types.DPosProposalVoteSlot) (bool, bool, error) {
	if _, exist := pool.GetConfirm(confirm.Hash); exist {
		return false, false, errors.New("duplicate confirm in pool")
	}

	// verify confirmation
	if err := blockchain.CheckConfirm(confirm); err != nil {
		return false, false, err
	}

	pool.Lock()
	if _, exist := pool.confirmMap[confirm.Hash]; exist {
		pool.Unlock()
		return false, false, errors.New("duplicate confirm in pool")
	}
	pool.confirmMap[confirm.Hash] = confirm
	pool.Unlock()

	inMainChain, isOrphan, err := pool.ConfirmBlock(confirm.Hash)
	if err != nil {
		return inMainChain, isOrphan, err
	}

	// notify arbiter new confirm received
	if blockchain.DefaultLedger.Blockchain.NewBlocksListeners != nil {
		for _, v := range blockchain.DefaultLedger.Blockchain.NewBlocksListeners {
			v.OnConfirmReceived(confirm)
		}
	}

	return inMainChain, isOrphan, nil
}

func (pool *BlockPool) ConfirmBlock(hash common.Uint256) (bool, bool, error) {
	log.Info("[ConfirmBlock] block hash:", hash)

	block, exist := pool.GetBlock(hash)
	if !exist {
		return false, false, errors.New("there is no block in pool when confirming block")
	}

	confirm, exist := pool.confirmMap[hash]
	if !exist {
		return false, false, errors.New("there is no block confirmation in pool when confirming block")
	}
	if err := blockchain.CheckBlockWithConfirmation(block, confirm); err != nil {
		return false, false, errors.New("block confirmation validate failed")
	}

	log.Info("[ConfirmBlock] block height:", block.Height)
	if !blockchain.DefaultLedger.Blockchain.BlockExists(&hash) {
		inMainChain, isOrphan, err := blockchain.DefaultLedger.Blockchain.AddBlock(block)
		if err != nil {
			return false, false, errors.New("add block failed," + err.Error())
		}

		if isOrphan || !inMainChain {
			return inMainChain, isOrphan, errors.New("add orphan block")
		}
	}

	err := blockchain.DefaultLedger.Blockchain.AddConfirm(confirm)
	if err != nil {
		return true, false, errors.New("add confirm failed")
	}
	return true, false, nil
}

func (pool *BlockPool) AddToBlockMap(block *types.Block) {
	pool.Lock()
	defer pool.Unlock()

	pool.blockMap[block.Hash()] = block
}

func (pool *BlockPool) GetBlock(hash common.Uint256) (*types.Block, bool) {
	pool.RLock()
	defer pool.RUnlock()

	block, ok := pool.blockMap[hash]
	return block, ok
}

func (pool *BlockPool) AddToConfirmMap(confirm *types.DPosProposalVoteSlot) {
	pool.Lock()
	defer pool.Unlock()

	pool.confirmMap[confirm.Hash] = confirm
}

func (pool *BlockPool) GetConfirm(hash common.Uint256) (*types.DPosProposalVoteSlot, bool) {
	pool.Lock()
	defer pool.Unlock()

	confirm, ok := pool.confirmMap[hash]
	return confirm, ok
}
