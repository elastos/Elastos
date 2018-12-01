package mempool

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/common/log"
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

func (pool *BlockPool) AppendBlock(blockConfirm *core.BlockConfirm) (bool, error) {
	log.Debugf("[AppendBlock] start")
	defer log.Debugf("[AppendBlock] end")

	// add block
	block := blockConfirm.Block
	hash := block.Hash()
	if _, exist := pool.GetBlock(hash); exist {
		return false, errors.New("duplicate block in pool")
	}
	// verify block
	if err := blockchain.PowCheckBlockSanity(block, config.Parameters.ChainParam.PowLimit, blockchain.DefaultLedger.Blockchain.TimeSource); err != nil {
		return false, err
	}
	pool.AddToBlockMap(block)

	// add confirm
	if blockConfirm.ConfirmFlag {
		err := pool.AppendConfirm(blockConfirm.Confirm)
		if err != nil {
			return false, err
		}
		return true, nil
	}

	// confirm block
	isConfirmed := true
	if err := pool.ConfirmBlock(hash); err != nil {
		log.Debug("[AppendBlock] ConfirmBlock failed, hash:", hash.String(), "err: ", err)
		isConfirmed = false
	}

	// notify arbiter new block received
	if blockchain.DefaultLedger.Blockchain.NewBlocksListeners != nil {
		for _, v := range blockchain.DefaultLedger.Blockchain.NewBlocksListeners {
			v.OnBlockReceived(block, isConfirmed)
		}
	}

	return isConfirmed, nil
}

func (pool *BlockPool) AppendConfirm(confirm *core.DPosProposalVoteSlot) error {
	// verify confirmation
	if err := blockchain.CheckConfirm(confirm); err != nil {
		return err
	}

	pool.Lock()
	if _, exist := pool.confirmMap[confirm.Hash]; exist {
		pool.Unlock()
		return errors.New("duplicate confirm in pool")
	}
	pool.confirmMap[confirm.Hash] = confirm
	pool.Unlock()

	if err := pool.ConfirmBlock(confirm.Hash); err != nil {
		return err
	}

	// notify arbiter new confirm received
	if blockchain.DefaultLedger.Blockchain.NewBlocksListeners != nil {
		for _, v := range blockchain.DefaultLedger.Blockchain.NewBlocksListeners {
			v.OnConfirmReceived(confirm)
		}
	}

	return nil
}

func (pool *BlockPool) ConfirmBlock(hash common.Uint256) error {
	log.Info("[ConfirmBlock] block hash:", hash)

	block, exist := pool.GetBlock(hash)
	if !exist {
		return errors.New("there is no block in pool when confirming block")
	}

	confirm, exist := pool.confirmMap[hash]
	if !exist {
		return errors.New("there is no block confirmation in pool when confirming block")
	}
	if err := blockchain.CheckBlockWithConfirmation(block, confirm); err != nil {
		return errors.New("block confirmation validate failed")
	}

	log.Info("[ConfirmBlock] block height:", block.Height)
	if !blockchain.DefaultLedger.Blockchain.BlockExists(&hash) {
		inMainChain, isOrphan, err := blockchain.DefaultLedger.Blockchain.AddBlock(block)
		if err != nil {
			return errors.New("add block failed," + err.Error())
		}

		if isOrphan || !inMainChain {
			return errors.New("add orphan block")
		}
	}

	err := blockchain.DefaultLedger.Blockchain.AddConfirm(confirm)
	if err != nil {
		return errors.New("add confirm failed")
	}
	return nil
}

func (pool *BlockPool) AddToBlockMap(block *core.Block) {
	pool.Lock()
	defer pool.Unlock()

	pool.blockMap[block.Hash()] = block
}

func (pool *BlockPool) GetBlock(hash common.Uint256) (*core.Block, bool) {
	pool.RLock()
	defer pool.RUnlock()

	block, ok := pool.blockMap[hash]
	return block, ok
}

func (pool *BlockPool) AddToConfirmMap(confirm *core.DPosProposalVoteSlot) {
	pool.Lock()
	defer pool.Unlock()

	pool.confirmMap[confirm.Hash] = confirm
}

func (pool *BlockPool) GetConfirm(hash common.Uint256) (*core.DPosProposalVoteSlot, bool) {
	pool.RLock()
	defer pool.RUnlock()

	confirm, ok := pool.confirmMap[hash]
	return confirm, ok
}
