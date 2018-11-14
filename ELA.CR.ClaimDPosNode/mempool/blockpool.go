package mempool

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
)

type BlockPool struct {
	sync.RWMutex
	blockCnt   uint64
	blockMap   map[common.Uint256]*core.Block
	confirmMap map[common.Uint256]*msg.DPosProposalVoteSlot
}

func (pool *BlockPool) Init() {
	pool.Lock()
	defer pool.Unlock()

	pool.blockMap = make(map[common.Uint256]*core.Block)
	pool.confirmMap = make(map[common.Uint256]*msg.DPosProposalVoteSlot)
}

func (pool *BlockPool) AppendBlock(block *core.Block) error {
	pool.Lock()
	defer pool.Unlock()

	hash := block.Hash()
	if _, exist := pool.blockMap[hash]; exist {
		return errors.New("duplicate block in pool")
	}
	// verify block
	if err := blockchain.PowCheckBlockSanity(block, config.Parameters.ChainParam.PowLimit, blockchain.DefaultLedger.Blockchain.TimeSource); err != nil {
		return err
	}
	pool.blockMap[hash] = block

	isConfirmed := true
	// confirm block
	if err := pool.ConfirmBlock(hash); err != nil {
		isConfirmed = false
		return err
	}

	// notify arbiter new block received
	blockchain.DefaultLedger.Blockchain.NewBlocksListener.OnBlockReceived(block, isConfirmed)

	return nil
}

func (pool *BlockPool) AppendConfirm(confirm *msg.DPosProposalVoteSlot) error {
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
	pool.Lock()
	defer pool.Unlock()

	block, exist := pool.blockMap[hash]
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
