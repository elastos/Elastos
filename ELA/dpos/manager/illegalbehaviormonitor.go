package manager

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
)

type illegalBehaviorMonitor struct {
	dispatcher *proposalDispatcher
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
