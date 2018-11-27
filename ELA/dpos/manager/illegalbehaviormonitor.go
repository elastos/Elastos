package manager

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type illegalBehaviorMonitor struct {
	dispatcher      *proposalDispatcher
	cachedProposals map[common.Uint256]*core.DPosProposal
}

func (i *illegalBehaviorMonitor) AddProposal(proposal core.DPosProposal) {
	i.cachedProposals[proposal.Hash()] = &proposal
}

func (i *illegalBehaviorMonitor) Reset() {
	i.cachedProposals = make(map[common.Uint256]*core.DPosProposal)
	for k, v := range i.dispatcher.pendingProposals {
		i.cachedProposals[k] = &v
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

	evidences := core.DposIllegalProposals{
		Evidence: core.ProposalEvidence{
			Proposal:    *first,
			BlockHeader: firstBlock.Header,
		},
		CompareEvidence: core.ProposalEvidence{
			Proposal:    *second,
			BlockHeader: secondBlock.Header,
		},
	}

	//todo send illegal proposal transaction

	m := &msg.IllegalProposals{Proposals: evidences}
	i.dispatcher.network.BroadcastMessage(m)
}

func (i *illegalBehaviorMonitor) ProcessIllegalVote(first, second *core.DPosProposalVote) {
	firstProposal, _ := i.cachedProposals[first.ProposalHash]
	secondProposal, _ := i.cachedProposals[second.ProposalHash]
	firstBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(firstProposal.BlockHash)
	secondBlock, _ := i.dispatcher.manager.GetBlockCache().TryGetValue(secondProposal.BlockHash)

	evidences := core.DposIllegalVotes{
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

	//todo send illegal vote transaction

	m := &msg.IllegalVotes{Votes: evidences}
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
