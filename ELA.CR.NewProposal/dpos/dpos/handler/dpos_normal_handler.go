package handler

import (
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type DposNormalHandler struct {
	*DposHandlerSwitch
}

func (h *DposNormalHandler) ProcessAcceptVote(p msg.DPosProposalVote) {
	log.Info("[Normal-ProcessAcceptVote] start")
	if h.consensus.IsArbitratorOnDuty(p.Proposal.Sponsor) {

		h.consensus.RunWithStatusCondition(h.consensus.IsRunning(), func() {
			if block, ok := ArbitratorSingleton.BlockCache.TryGetValue(p.Proposal.BlockHash); ok {
				h.proposalDispatcher.TryStartSpeculatingProposal(block)
			}

			h.proposalDispatcher.ProcessVote(p, true)
		})
	}
}

func (h *DposNormalHandler) ProcessRejectVote(p msg.DPosProposalVote) {
	if h.consensus.IsArbitratorOnDuty(p.Proposal.Sponsor) {

		h.consensus.RunWithStatusCondition(h.consensus.IsRunning(), func() {
			if block, ok := ArbitratorSingleton.BlockCache.TryGetValue(p.Proposal.BlockHash); ok {
				h.proposalDispatcher.TryStartSpeculatingProposal(block)
			}

			h.proposalDispatcher.ProcessVote(p, false)
		})
	}
}

func (h *DposNormalHandler) StartNewProposal(p msg.DPosProposal) {
	log.Info("[Normal][OnProposalReceived] received request sign")
	h.consensus.RunWithStatusCondition(true, func() {
		h.consensus.TryChangeView()
		h.proposalDispatcher.ProcessProposal(p)
	})
}

func (h *DposNormalHandler) ChangeView(firstBlockHash *common.Uint256) {
	log.Info("[OnViewChanged] clean proposal")
	h.consensus.RunWithStatusCondition(true, func() {
		h.proposalDispatcher.CleanProposals()
	})
}

func (h *DposNormalHandler) TryStartNewConsensus(b *core.Block) bool {
	result := false

	h.consensus.RunWithAllStatusConditions(
		func() { //ready
			log.Info("[Normal][OnBlockReceived] received first unsigned block, start consensus")
			h.consensus.StartConsensus(b)
			result = true
		},
		func() { //running
			log.Info("[Normal][OnBlockReceived] received unsigned block, record block")
			h.consensus.ProcessBlock(b)
		},
	)

	return result
}
