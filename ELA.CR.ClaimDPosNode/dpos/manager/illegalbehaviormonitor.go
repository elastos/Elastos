package manager

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/node"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const WaitHeightTolerance = uint32(1)

type IllegalBehaviorMonitor interface {
	IsBlockValid(block *core.Block) bool

	AddProposal(proposal core.DPosProposal)
	Reset(changeView bool)

	IsLegalProposal(p *core.DPosProposal) (*core.DPosProposal, bool)
	ProcessIllegalProposal(first, second *core.DPosProposal)

	ProcessIllegalVote(first, second *core.DPosProposalVote)
	IsLegalVote(v *core.DPosProposalVote) (*core.DPosProposalVote, bool)

	SetProposalEvidence(evidence *core.DposIllegalProposals)
	SetVoteEvidence(evidence *core.DposIllegalVotes)
}

type illegalBehaviorMonitor struct {
	dispatcher      *proposalDispatcher
	cachedProposals map[common.Uint256]*core.DPosProposal

	//todo consider multiple arbitrators have illegal evidence within same block
	proposalEvidence *core.DposIllegalProposals
	voteEvidence     *core.DposIllegalVotes
}

func (i *illegalBehaviorMonitor) SetProposalEvidence(evidence *core.DposIllegalProposals) {
	i.proposalEvidence = evidence
}

func (i *illegalBehaviorMonitor) SetVoteEvidence(evidence *core.DposIllegalVotes) {
	i.voteEvidence = evidence
}

func (i *illegalBehaviorMonitor) IsBlockValid(block *core.Block) bool {
	if i.proposalEvidence != nil {
		for _, t := range block.Transactions {
			if t.IsIllegalProposalTx() {
				payload := t.Payload.(*core.PayloadIllegalProposal)
				if payload.DposIllegalProposals.Hash().IsEqual(i.proposalEvidence.Hash()) {
					return true
				}
			}
		}

		if block.Height-i.proposalEvidence.Evidence.BlockHeader.Height > WaitHeightTolerance {
			return false
		}
	}

	if i.voteEvidence != nil {
		for _, t := range block.Transactions {
			payload := t.Payload.(*core.PayloadIllegalVote)
			if payload.DposIllegalVotes.Hash().IsEqual(i.voteEvidence.Hash()) {
				return true
			}
		}

		if block.Height-i.voteEvidence.Evidence.BlockHeader.Height > WaitHeightTolerance {
			return false
		}
	}

	return true
}

func (i *illegalBehaviorMonitor) AddProposal(proposal core.DPosProposal) {
	i.cachedProposals[proposal.Hash()] = &proposal
}

func (i *illegalBehaviorMonitor) Reset(changeView bool) {
	i.cachedProposals = make(map[common.Uint256]*core.DPosProposal)
	for k, v := range i.dispatcher.pendingProposals {
		i.cachedProposals[k] = &v
	}

	if !changeView && i.dispatcher.processingBlock != nil {
		i.proposalEvidence = nil

		if i.IsBlockValid(i.dispatcher.processingBlock) {
			i.voteEvidence = nil
		}
	}
}

func (i *illegalBehaviorMonitor) IsLegalProposal(p *core.DPosProposal) (*core.DPosProposal, bool) {
	if i.isProposalsIllegal(p, i.dispatcher.processingProposal) {
		return i.dispatcher.processingProposal, false
	}

	for _, pending := range i.dispatcher.pendingProposals {
		if i.isProposalsIllegal(p, &pending) {
			return &pending, false
		}
	}

	return nil, true
}

func (i *illegalBehaviorMonitor) ProcessIllegalProposal(first, second *core.DPosProposal) {
	firstBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(first.BlockHash)
	secondBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(second.BlockHash)

	evidences := &core.DposIllegalProposals{
		Evidence: core.ProposalEvidence{
			Proposal:    *first,
			BlockHeader: firstBlock.Header,
		},
		CompareEvidence: core.ProposalEvidence{
			Proposal:    *second,
			BlockHeader: secondBlock.Header,
		},
	}

	i.SetProposalEvidence(evidences)
	i.sendIllegalProposalTransaction(evidences)

	m := &msg.IllegalProposals{Proposals: *evidences}
	i.dispatcher.network.BroadcastMessage(m)
}

func (i *illegalBehaviorMonitor) sendIllegalProposalTransaction(evidences *core.DposIllegalProposals) {
	transaction := &core.Transaction{
		Version:        core.TransactionVersion(blockchain.DefaultLedger.HeightVersions.GetDefaultTxVersion(i.dispatcher.processingBlock.Height)),
		TxType:         core.IllegalProposalEvidence,
		PayloadVersion: core.PayloadIllegalProposalVersion,
		Payload:        &core.PayloadIllegalProposal{DposIllegalProposals: *evidences},
		Attributes:     []*core.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*core.Output{},
		Inputs:         []*core.Input{},
		Fee:            0,
	}

	if code := node.LocalNode.AppendToTxnPool(transaction); code == errors.Success {
		node.LocalNode.Relay(nil, transaction)
	}
}

func (i *illegalBehaviorMonitor) sendIllegalVoteTransaction(evidences *core.DposIllegalVotes) {
	transaction := &core.Transaction{
		Version:        core.TransactionVersion(blockchain.DefaultLedger.HeightVersions.GetDefaultTxVersion(i.dispatcher.processingBlock.Height)),
		TxType:         core.IllegalVoteEvidence,
		PayloadVersion: core.PayloadIllegalVoteVersion,
		Payload:        &core.PayloadIllegalVote{DposIllegalVotes: *evidences},
		Attributes:     []*core.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*core.Output{},
		Inputs:         []*core.Input{},
		Fee:            0,
	}

	if code := node.LocalNode.AppendToTxnPool(transaction); code == errors.Success {
		node.LocalNode.Relay(nil, transaction)
	}
}

func (i *illegalBehaviorMonitor) ProcessIllegalVote(first, second *core.DPosProposalVote) {
	firstProposal, _ := i.cachedProposals[first.ProposalHash]
	secondProposal, _ := i.cachedProposals[second.ProposalHash]
	firstBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(firstProposal.BlockHash)
	secondBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(secondProposal.BlockHash)

	evidences := &core.DposIllegalVotes{
		Evidence: core.VoteEvidence{
			Vote:        *first,
			Proposal:    *firstProposal,
			BlockHeader: firstBlock.Header,
		},
		CompareEvidence: core.VoteEvidence{
			Vote:        *second,
			Proposal:    *secondProposal,
			BlockHeader: secondBlock.Header,
		},
	}

	i.SetVoteEvidence(evidences)
	i.sendIllegalVoteTransaction(evidences)

	m := &msg.IllegalVotes{Votes: *evidences}
	i.dispatcher.network.BroadcastMessage(m)
}

func (i *illegalBehaviorMonitor) isProposalsIllegal(first, second *core.DPosProposal) bool {
	if first == nil || second == nil {
		return false
	}

	if first.Sponsor != second.Sponsor || first.ViewOffset != second.ViewOffset {
		return false
	}

	firstBlock, foundFirst := i.dispatcher.manager.GetBlockCache().TryGetValue(first.BlockHash)
	secondBlock, foundSecond := i.dispatcher.manager.GetBlockCache().TryGetValue(second.BlockHash)
	if !foundFirst || !foundSecond {
		return false
	}

	if firstBlock.Height == secondBlock.Height {
		return true
	}

	return false
}

func (i *illegalBehaviorMonitor) IsLegalVote(v *core.DPosProposalVote) (*core.DPosProposalVote, bool) {
	for _, accept := range i.dispatcher.acceptVotes {
		if i.isVotesIllegal(v, &accept) {
			return &accept, false
		}
	}

	for _, reject := range i.dispatcher.rejectedVotes {
		if i.isVotesIllegal(v, &reject) {
			return &reject, false
		}
	}

	return nil, true
}

func (i *illegalBehaviorMonitor) isVotesIllegal(first, second *core.DPosProposalVote) bool {
	if first.Signer != second.Signer {
		return false
	}

	if first.ProposalHash.IsEqual(second.ProposalHash) {
		return !first.Hash().IsEqual(second.Hash())
	}

	firstProposal, foundFirst := i.cachedProposals[first.ProposalHash]
	secondProposal, foundSecond := i.cachedProposals[second.ProposalHash]
	if !foundFirst || !foundSecond {
		return false
	}

	return i.isProposalsIllegal(firstProposal, secondProposal)
}
