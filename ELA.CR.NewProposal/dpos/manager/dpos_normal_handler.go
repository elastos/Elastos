package manager

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type DposNormalHandler struct {
	*dposHandlerSwitch
}

func (h *DposNormalHandler) ProcessAcceptVote(id peer.PID, p core.DPosProposalVote) {
	log.Info("[Normal-ProcessAcceptVote] start")
	defer log.Info("[Normal-ProcessAcceptVote] end")

	if !h.consensus.IsRunning() {
		return
	}

	currentProposal, ok := h.tryGetCurrentProposal(id, p)
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		h.proposalDispatcher.ProcessVote(p, true)
	}
}

func (h *DposNormalHandler) ProcessRejectVote(id peer.PID, p core.DPosProposalVote) {
	log.Info("[Normal-ProcessRejectVote] start")
	if !h.consensus.IsRunning() {
		return
	}

	currentProposal, ok := h.tryGetCurrentProposal(id, p)
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		h.proposalDispatcher.ProcessVote(p, false)
	}
}

func (h *DposNormalHandler) tryGetCurrentProposal(id peer.PID, p core.DPosProposalVote) (common.Uint256, bool) {
	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal == nil {
		requestProposal := &msg.RequestProposal{ProposalHash: p.ProposalHash}
		h.network.SendMessageToPeer(id, requestProposal)
		return common.Uint256{}, false
	}
	return currentProposal.Hash(), true
}

func (h *DposNormalHandler) StartNewProposal(p core.DPosProposal) {
	log.Info("[Normal][StartNewProposal] start")
	defer log.Info("[Normal][StartNewProposal] end")

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
