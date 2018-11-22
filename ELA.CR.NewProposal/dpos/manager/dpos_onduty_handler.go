package manager

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type DposOnDutyHandler struct {
	*dposHandlerSwitch
}

func (h *DposOnDutyHandler) ProcessAcceptVote(id peer.PID, p core.DPosProposalVote) {
	log.Info("[Onduty-ProcessAcceptVote] start")

	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal != nil && currentProposal.Hash().IsEqual(p.ProposalHash) && h.consensus.IsRunning() {
		log.Info("[OnVoteReceived] Received needed sign, collect it")
		h.proposalDispatcher.ProcessVote(p, true)
	}
}

func (h *DposOnDutyHandler) ProcessRejectVote(id peer.PID, p core.DPosProposalVote) {
	log.Info("[Onduty-ProcessRejectVote] start")

	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal != nil && currentProposal.Hash().IsEqual(p.ProposalHash) && h.consensus.IsRunning() {
		h.proposalDispatcher.ProcessVote(p, false)
	}
}

func (h *DposOnDutyHandler) StartNewProposal(p core.DPosProposal) {
}

func (h *DposOnDutyHandler) ChangeView(firstBlockHash *common.Uint256) {
	b, ok := h.manager.GetBlockCache().TryGetValue(*firstBlockHash)
	if !ok {
		log.Info("[OnViewChanged] get block failed for proposal")
	} else {
		log.Info("[OnViewChanged] start proposal")
		h.proposalDispatcher.CleanProposals()
		h.proposalDispatcher.StartProposal(b)
	}
}

func (h *DposOnDutyHandler) TryStartNewConsensus(b *core.Block) bool {
	result := false

	if h.consensus.IsReady() {
		log.Info("[OnDuty][OnBlockReceived] received first unsigned block, start consensus")
		h.consensus.StartConsensus(b)
		h.proposalDispatcher.StartProposal(b)
		result = true
	} else { //finished
		log.Info("[OnDuty][OnBlockReceived] received unsigned block, do nothing")
		h.consensus.ProcessBlock(b)
		result = false
	}

	return result
}
