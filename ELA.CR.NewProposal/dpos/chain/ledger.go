package chain

import (
	"errors"

	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Ledger struct {
	BlockMap        map[common.Uint256]*Block
	BlockConfirmMap map[common.Uint256]*ProposalVoteSlot
	LastBlock       *Block
	GenesisBlock    *Block

	PendingBlockConfirms map[common.Uint256]*ProposalVoteSlot

	backup *Ledger
}

func (l *Ledger) AppendPendingConfirms(b *ProposalVoteSlot) {
	l.PendingBlockConfirms[b.Hash] = b
}

func (l *Ledger) GetPendingConfirms(blockHash common.Uint256) (*ProposalVoteSlot, bool) {
	confirm, ok := l.PendingBlockConfirms[blockHash]
	return confirm, ok
}

func (l *Ledger) TryAppendBlock(b *Block, p *ProposalVoteSlot) bool {
	return l.appendBlockInner(b, p, false)
}

func (l *Ledger) GetBlocksAndConfirms(start, end uint32) ([]*Block, []*ProposalVoteSlot, error) {
	if l.LastBlock == nil || l.LastBlock.Height < start {
		return nil, nil, errors.New("Result empty")
	}

	blocks := make([]*Block, 0)
	blockConfirms := make([]*ProposalVoteSlot, 0)

	//todo improve when merge into arbitrator
	for k, v := range l.BlockMap {
		if v.Height >= start && (end == 0 || v.Height <= end) {
			blocks = append(blocks, v)

			if confirm, ok := l.BlockConfirmMap[k]; ok {
				blockConfirms = append(blockConfirms, confirm)
			} else {
				return nil, nil, errors.New("Can not find block related confirm, block hash: " + k.String())
			}
		}
	}

	return blocks, blockConfirms, nil
}

func (l *Ledger) Restore() {
	l.backup = &Ledger{}
	l.backup.GenesisBlock = l.GenesisBlock
	l.backup.LastBlock = l.LastBlock

	l.backup.BlockMap = make(map[common.Uint256]*Block)
	for k, v := range l.BlockMap {
		l.backup.BlockMap[k] = v
	}

	l.backup.BlockConfirmMap = make(map[common.Uint256]*ProposalVoteSlot)
	for k, v := range l.BlockConfirmMap {
		l.backup.BlockConfirmMap[k] = v
	}
}

func (l *Ledger) Rollback() error {
	if l.backup == nil {
		return errors.New("Can not rollback")
	}

	l.GenesisBlock = l.backup.GenesisBlock
	l.LastBlock = l.backup.LastBlock

	l.BlockMap = make(map[common.Uint256]*Block)
	for k, v := range l.backup.BlockMap {
		l.BlockMap[k] = v
	}

	l.BlockConfirmMap = make(map[common.Uint256]*ProposalVoteSlot)
	for k, v := range l.backup.BlockConfirmMap {
		l.BlockConfirmMap[k] = v
	}
	return nil
}

func (l *Ledger) CollectConsensusStatus(height uint32, missingBlocks []*Block, missingBlockConfirms []*ProposalVoteSlot) error {
	//todo limit max blocks count for collecting
	var err error
	if missingBlocks, missingBlockConfirms, err = l.GetBlocksAndConfirms(height, 0); err != nil {
		return err
	}
	return nil
}

func (l *Ledger) RecoverFromConsensusStatus(missingBlocks []*Block, missingBlockConfirms []*ProposalVoteSlot) error {
	for i := range missingBlocks {
		if !l.appendBlockInner(missingBlocks[i], missingBlockConfirms[i], true) {
			return errors.New("Append block error")
		}
	}
	return nil
}

func (l *Ledger) appendBlockInner(b *Block, p *ProposalVoteSlot, ignoreIfExist bool) bool {
	if b == nil {
		log.Info("Block is nil")
		return false
	}

	if p == nil {
		log.Info("ProposalVoteSlot is nil")
		return false
	}

	if !IsValidBlock(b, p) {
		log.Info("Invalid block")
		return false
	}

	if _, ok := l.BlockMap[b.Hash]; ok {
		log.Info("Already has block")
		return ignoreIfExist
	}

	if l.LastBlock == nil || b.Height > l.LastBlock.Height {
		l.BlockMap[b.Hash] = b
		l.BlockConfirmMap[b.Hash] = p
		l.LastBlock = b
		return true
	}
	return false
}

func IsValidBlock(b *Block, p *ProposalVoteSlot) bool {
	if !b.Hash.IsEqual(p.Hash) {
		log.Info("Received block is not current processing block")
		return false
	}
	//todo replace 3 with 2/3 of real arbitrators count
	log.Info("len signs:", len(p.Votes))
	return len(p.Votes) >= 3
}
