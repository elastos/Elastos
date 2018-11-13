package handler

import (
	"github.com/elastos/Elastos.ELA/core"
	common2 "github.com/elastos/Elastos.ELA/dpos/arbitration/common"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

type DposOnDutyHandler struct {
	*DposHandlerSwitch
}

func (h *DposOnDutyHandler) ProcessAcceptVote(p common2.DPosProposalVote) {
	log.Info("[Onduty-ProcessAcceptVote] start")
	if h.consensus.IsArbitratorOnDuty(p.Proposal.Sponsor) {

		h.consensus.RunWithStatusCondition(h.consensus.IsRunning(), func() {
			log.Info("[OnVoteReceived] Received needed sign, collect it")
			h.proposalDispatcher.ProcessVote(p, true)
		})
	}
}

func (h *DposOnDutyHandler) ProcessRejectVote(p common2.DPosProposalVote) {
	if h.consensus.IsArbitratorOnDuty(p.Proposal.Sponsor) {

		h.consensus.RunWithStatusCondition(h.consensus.IsRunning(), func() {
			h.proposalDispatcher.ProcessVote(p, false)
		})
	}
}

func (h *DposOnDutyHandler) StartNewProposal(p common2.DPosProposal) {
}

func (h *DposOnDutyHandler) ChangeView(firstBlockHash *common.Uint256) {
	b, ok := ArbitratorSingleton.BlockCache.TryGetValue(*firstBlockHash)
	if !ok {
		log.Info("[OnViewChanged] get block failed for proposal")
	} else {

		h.consensus.RunWithStatusCondition(true, func() {
			log.Info("[OnViewChanged] start proposal")
			h.proposalDispatcher.CleanProposals()
			h.proposalDispatcher.StartProposal(b)
		})
	}
}

func (h *DposOnDutyHandler) TryStartNewConsensus(peer *peer.Peer, b *core.Block) bool {
	result := false

	h.consensus.RunWithAllStatusConditions(func() { //ready
		log.Info("[OnDuty][OnBlockReceived] received first unsigned block, start consensus")
		h.consensus.StartConsensus(b)
		h.proposalDispatcher.StartProposal(b)
		result = true
	}, func() {
		log.Info("[OnDuty][OnBlockReceived] received unsigned block, do nothing")
		h.consensus.ProcessBlock(b)
		result = false
	})

	return result
}
