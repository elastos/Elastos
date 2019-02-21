package mempool

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/events"
)

type BlockPool struct {
	Chain *blockchain.BlockChain

	sync.RWMutex
	blocks   map[common.Uint256]*types.Block
	confirms map[common.Uint256]*payload.Confirm
}

func (bm *BlockPool) AppendDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	bm.Lock()
	inMainChain, isOrphan, err := bm.appendDposBlock(dposBlock)
	bm.Unlock()
	return inMainChain, isOrphan, err
}

func (bm *BlockPool) appendDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	// add block
	block := dposBlock.Block
	hash := block.Hash()
	if _, ok := bm.blocks[hash]; ok {
		return false, false, errors.New("duplicate block in pool")
	}
	// verify block
	if err := bm.Chain.CheckBlockSanity(block); err != nil {
		return false, false, err
	}
	bm.blocks[block.Hash()] = block

	// add confirm
	if dposBlock.ConfirmFlag {
		bm.appendConfirm(dposBlock.Confirm)
	}

	// confirm block
	copyBlock := *dposBlock
	copyBlock.ConfirmFlag = true
	inMainChain, isOrphan, err := bm.confirmBlock(hash)
	if err != nil {
		log.Debug("[AppendDposBlock] ConfirmBlock failed, hash:", hash.String(), "err: ", err)
		copyBlock.ConfirmFlag = false

		// Notify the caller that the new block without confirm was accepted.
		// The caller would typically want to react by relaying the inventory
		// to other peers.
		events.Notify(events.ETBlockAccepted, block)
	}

	// notify new block received
	events.Notify(events.ETNewBlockReceived, &copyBlock)

	return inMainChain, isOrphan, nil
}

func (bm *BlockPool) AppendConfirm(confirm *payload.Confirm) (bool,
	bool, error) {
	bm.Lock()
	inMainChain, isOrphan, err := bm.appendConfirm(confirm)
	bm.Unlock()
	return inMainChain, isOrphan, err
}

func (bm *BlockPool) appendConfirm(confirm *payload.Confirm) (
	bool, bool, error) {
	if _, ok := bm.confirms[confirm.Hash]; ok {
		return false, false, errors.New("duplicate confirm in pool")
	}

	// verify confirmation
	if err := blockchain.ConfirmSanityCheck(confirm); err != nil {
		return false, false, err
	}
	bm.confirms[confirm.Hash] = confirm

	inMainChain, isOrphan, err := bm.confirmBlock(confirm.Hash)
	if err != nil {
		return inMainChain, isOrphan, err
	}

	// notify new confirm accepted.
	events.Notify(events.ETConfirmAccepted, confirm)

	return inMainChain, isOrphan, nil
}

func (bm *BlockPool) ConfirmBlock(hash common.Uint256) (bool, bool, error) {
	bm.Lock()
	inMainChain, isOrphan, err := bm.confirmBlock(hash)
	bm.Unlock()
	return inMainChain, isOrphan, err
}

func (bm *BlockPool) confirmBlock(hash common.Uint256) (bool, bool, error) {
	log.Info("[ConfirmBlock] block hash:", hash)

	block, ok := bm.blocks[hash]
	if !ok {
		return false, false, errors.New("there is no block in pool when confirming block")
	}

	confirm, ok := bm.confirms[hash]
	if !ok {
		return false, false, errors.New("there is no block confirmation in pool when confirming block")
	}
	if err := blockchain.CheckBlockWithConfirmation(block, confirm); err != nil {
		return false, false, errors.New("block confirmation validate failed")
	}

	log.Info("[ConfirmBlock] block height:", block.Height)
	if !bm.Chain.BlockExists(&hash) {
		inMainChain, isOrphan, err := bm.Chain.ProcessBlock(block)
		if err != nil {
			return inMainChain, isOrphan, errors.New("add block failed," + err.Error())
		}

		if isOrphan || !inMainChain {
			return inMainChain, isOrphan, errors.New("add orphan block")
		}
	}

	err := bm.Chain.AddConfirm(confirm)
	if err != nil {
		return true, false, errors.New("add confirm failed")
	}
	return true, false, nil
}

func (bm *BlockPool) AddToBlockMap(block *types.Block) {
	bm.Lock()
	defer bm.Unlock()

	bm.blocks[block.Hash()] = block
}

func (bm *BlockPool) GetBlock(hash common.Uint256) (*types.Block, bool) {
	bm.RLock()
	defer bm.RUnlock()

	block, ok := bm.blocks[hash]
	return block, ok
}

func (bm *BlockPool) GetDposBlockByHash(hash common.Uint256) (*types.DposBlock, error) {
	bm.RLock()
	defer bm.RUnlock()

	block, ok := bm.blocks[hash]
	if !ok {
		return nil, errors.New("not found block")
	}

	return &types.DposBlock{
		BlockFlag: true,
		Block:     block,
	}, nil
}

func (bm *BlockPool) AddToConfirmMap(confirm *payload.Confirm) {
	bm.Lock()
	defer bm.Unlock()

	bm.confirms[confirm.Hash] = confirm
}

func (bm *BlockPool) GetConfirm(hash common.Uint256) (
	*payload.Confirm, bool) {
	bm.Lock()
	defer bm.Unlock()

	confirm, ok := bm.confirms[hash]
	return confirm, ok
}

func NewBlockPool() *BlockPool {
	return &BlockPool{
		blocks:   make(map[common.Uint256]*types.Block),
		confirms: make(map[common.Uint256]*payload.Confirm),
	}
}
