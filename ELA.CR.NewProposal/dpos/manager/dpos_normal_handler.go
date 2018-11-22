package manager

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type DposNormalHandler struct {
	*dposHandlerSwitch
}

func (h *DposNormalHandler) ProcessAcceptVote(p core.DPosProposalVote) {
	log.Info("[Normal-ProcessAcceptVote] start")
	if !h.consensus.IsRunning() {
		return
	}

	currentProposal, ok := h.tryGetCurrentProposal()
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		h.proposalDispatcher.ProcessVote(p, true)
	}
}

func (h *DposNormalHandler) ProcessRejectVote(p core.DPosProposalVote) {
	log.Info("[Normal-ProcessRejectVote] start")
	if !h.consensus.IsRunning() {
		return
	}

	currentProposal, ok := h.tryGetCurrentProposal()
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		h.proposalDispatcher.ProcessVote(p, false)
	}
}

func (h *DposNormalHandler) tryGetCurrentProposal() (common.Uint256, bool) {
	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal == nil {
		//todo request proposal from vote sender
		return common.Uint256{}, false
	}
	return currentProposal.Hash(), true
}

func (h *DposNormalHandler) StartNewProposal(p core.DPosProposal) {
	log.Info("[Normal][OnProposalReceived] received request sign")
	if h.consensus.IsRunning() {
		h.consensus.TryChangeView()
	}

	h.proposalDispatcher.ProcessProposal(p)
}

func (h *DposNormalHandler) ChangeView(firstBlockHash *common.Uint256) {
	log.Info("[OnViewChanged] clean proposal")
	h.proposalDispatcher.CleanProposals()
}

func (h *DposNormalHandler) TryStartNewConsensus(b *core.Block) bool {
	result := false

	if h.consensus.IsReady() {
		log.Info("[Normal][OnBlockReceived] received first unsigned block, start consensus")
		h.proposalDispatcher.CleanProposals()
		h.consensus.StartConsensus(b)
		h.proposalDispatcher.TryStartSpeculatingProposal(b)
		result = true
	} else { //running
		log.Info("[Normal][OnBlockReceived] received unsigned block, record block")
		h.consensus.ProcessBlock(b)
	}

	return result
}
