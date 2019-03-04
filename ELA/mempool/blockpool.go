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

func (bm *BlockPool) AppendConfirm(confirm *payload.Confirm) (bool,
	bool, error) {
	bm.Lock()
	defer bm.Unlock()

	inMainChain, isOrphan, err := bm.appendConfirm(confirm)
	if err != nil {
		return inMainChain, isOrphan, err
	}

	if block, ok := bm.blocks[confirm.Proposal.Hash()]; ok {
		events.Notify(events.ETBlockAccepted, block)
	}

	return inMainChain, isOrphan, err
}

func (bm *BlockPool) AppendDposBlock(dposBlock *types.DposBlock) (bool, bool, error) {
	bm.Lock()
	defer bm.Unlock()
	if !dposBlock.ConfirmFlag {
		return bm.appendBlock(dposBlock)
	}
	return bm.appendBlockAndConfirm(dposBlock)
}

func (bm *BlockPool) appendBlock(dposBlock *types.DposBlock) (bool, bool, error) {
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

	// confirm block
	inMainChain, isOrphan, err := bm.confirmBlock(hash)
	if err != nil {
		log.Debug("[AppendDposBlock] ConfirmBlock failed, hash:", hash.String(), "err: ", err)

		// Notify the caller that the new block without confirm was accepted.
		// The caller would typically want to react by relaying the inventory
		// to other peers.
		events.Notify(events.ETBlockAccepted, block)
		events.Notify(events.ETNewBlockReceived, dposBlock)
		return inMainChain, isOrphan, nil
	}

	copyBlock := *dposBlock
	confirm := bm.confirms[hash]
	copyBlock.ConfirmFlag = confirm != nil
	copyBlock.Confirm = confirm

	// notify new block received
	events.Notify(events.ETNewBlockReceived, &copyBlock)

	return inMainChain, isOrphan, nil
}

func (bm *BlockPool) appendBlockAndConfirm(dposBlock *types.DposBlock) (bool, bool, error) {
	block := dposBlock.Block
	hash := block.Hash()
	// verify block
	if err := bm.Chain.CheckBlockSanity(block); err != nil {
		return false, false, err
	}
	// add block
	bm.blocks[block.Hash()] = block
	// confirm block
	inMainChain, isOrphan, err := bm.appendConfirm(dposBlock.Confirm)
	if err != nil {
		log.Debug("[appendBlockAndConfirm] ConfirmBlock failed, hash:", hash.String(), "err: ", err)
		return inMainChain, isOrphan, nil
	}

	// notify new block received
	events.Notify(events.ETNewBlockReceived, dposBlock)

	return inMainChain, isOrphan, nil
}

func (bm *BlockPool) appendConfirm(confirm *payload.Confirm) (
	bool, bool, error) {
	if _, ok := bm.confirms[confirm.Proposal.BlockHash]; ok {
		return false, true, errors.New("duplicate confirm in pool")
	}

	// verify confirmation
	if err := blockchain.ConfirmSanityCheck(confirm); err != nil {
		return false, false, err
	}
	bm.confirms[confirm.Proposal.BlockHash] = confirm

	inMainChain, isOrphan, err := bm.confirmBlock(confirm.Proposal.BlockHash)
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

	log.Info("[ConfirmBlock] block height:", block.Height)
	if !bm.Chain.BlockExists(&hash) {
		inMainChain, isOrphan, err := bm.Chain.ProcessBlock(block, confirm)
		if err != nil {
			return inMainChain, isOrphan, errors.New("add block failed," + err.Error())
		}

		if isOrphan && !inMainChain {
			bm.Chain.AddOrphanConfirm(confirm)
		}

		if isOrphan || !inMainChain {
			return inMainChain, isOrphan, errors.New("add orphan block")
		}
	} else {
		return false, false, errors.New("already processed block")
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

	if block, _ := bm.Chain.GetDposBlockByHash(hash); block != nil {
		return block, nil
	}

	if block := bm.blocks[hash]; block != nil {
		confirm := bm.confirms[hash]
		return &types.DposBlock{
			BlockFlag:   true,
			Block:       block,
			ConfirmFlag: confirm != nil,
			Confirm:     confirm,
		}, nil
	}
	return nil, errors.New("not found dpos block")
}

func (bm *BlockPool) AddToConfirmMap(confirm *payload.Confirm) {
	bm.Lock()
	defer bm.Unlock()

	bm.confirms[confirm.Proposal.BlockHash] = confirm
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
