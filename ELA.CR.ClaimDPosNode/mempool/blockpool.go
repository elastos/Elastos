package mempool

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockPool struct {
	sync.RWMutex
	blockCnt   uint64
	blockMap   map[common.Uint256]*core.Block
	confirmMap map[common.Uint256]*core.DPosProposalVoteSlot
}

func (pool *BlockPool) Init() {
	pool.Lock()
	defer pool.Unlock()

	pool.blockMap = make(map[common.Uint256]*core.Block)
	pool.confirmMap = make(map[common.Uint256]*core.DPosProposalVoteSlot)
}

func (pool *BlockPool) AppendBlock(block *core.Block) error {

	hash := block.Hash()
	if _, exist := pool.GetBlock(hash); exist {
		return errors.New("duplicate block in pool")
	}
	// verify block
	if err := blockchain.PowCheckBlockSanity(block, config.Parameters.ChainParam.PowLimit, blockchain.DefaultLedger.Blockchain.TimeSource); err != nil {
		return err
	}
	if ok := pool.AddToBlockMap(block); !ok {
		return errors.New("duplicate block in pool")
	}

	isConfirmed := true
	// confirm block
	if err := pool.ConfirmBlock(hash); err != nil {
		isConfirmed = false
		return err
	}

	// notify arbiter new block received
	if blockchain.DefaultLedger.Blockchain.NewBlocksListener != nil {
		blockchain.DefaultLedger.Blockchain.NewBlocksListener.OnBlockReceived(block, isConfirmed)
	}

	return nil
}

func (pool *BlockPool) AppendConfirm(confirm *core.DPosProposalVoteSlot) error {
	pool.Lock()
	defer pool.Unlock()

	if _, exist := pool.confirmMap[confirm.Hash]; exist {
		return errors.New("duplicate confirm in pool")
	}

	// verify confirmation
	if err := blockchain.CheckConfirm(confirm); err != nil {
		return err
	}
	pool.confirmMap[confirm.Hash] = confirm
	// notify arbiter new confirm received
	blockchain.DefaultLedger.Blockchain.NewBlocksListener.OnConfirmReceived(confirm)

	if err := pool.ConfirmBlock(confirm.Hash); err != nil {
		return err
	}

	return nil
}

func (pool *BlockPool) ConfirmBlock(hash common.Uint256) error {
	block, exist := pool.GetBlock(hash)
	if !exist {
		return errors.New("there is no block in pool when confirming block")
	}

	// FIXME use chain params
	if config.Parameters.ChainParam.Name != "RegNet" {
		confirm, exist := pool.confirmMap[hash]
		if !exist {
			return errors.New("there is no block confirmation in pool when confirming block")
		}
		if err := blockchain.CheckBlockWithConfirmation(block, confirm); err != nil {
			return errors.New("block confirmation validate failed")
		}
	}

	inMainChain, isOrphan, err := blockchain.DefaultLedger.Blockchain.AddBlock(block)
	if err != nil {
		return errors.New("add block failed")
	}

	if isOrphan || !inMainChain {
		return errors.New("add orphan block")
	}

	return nil
}

func (pool *BlockPool) AddToBlockMap(block *core.Block) bool {
	pool.Lock()
	defer pool.Unlock()

	hash := block.Hash()
	if _, ok := pool.blockMap[hash]; ok {
		return false
	}
	pool.blockMap[hash] = block

	return true
}

func (pool *BlockPool) GetBlock(hash common.Uint256) (*core.Block, bool) {
	pool.RLock()
	defer pool.RUnlock()

	block, ok := pool.blockMap[hash]
	return block, ok
}

func (pool *BlockPool) AddToConfirmMap(confirm *core.DPosProposalVoteSlot) error {

	return nil
}

func (pool *BlockPool) GetConfirm(hash common.Uint256) (*core.DPosProposalVoteSlot, error) {
	pool.Lock()
	defer pool.Unlock()

	confirm, exist := pool.confirmMap[hash]
	if !exist {
		return nil, errors.New("not found confirm")
	}

	return confirm, nil
}
