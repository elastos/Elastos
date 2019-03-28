package manager

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
)

type DPOSNormalHandler struct {
	*DPOSHandlerSwitch
}

func (h *DPOSNormalHandler) ProcessAcceptVote(id peer.PID, p *payload.DPOSProposalVote) (succeed bool, finished bool) {
	log.Info("[Normal-ProcessAcceptVote] start")
	defer log.Info("[Normal-ProcessAcceptVote] end")

	if !h.consensus.IsRunning() {
		return false, false
	}

	currentProposal, ok := h.tryGetCurrentProposal(id, p)
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		return h.proposalDispatcher.ProcessVote(p, true)
	}

	return false, false
}

func (h *DPOSNormalHandler) ProcessRejectVote(id peer.PID, p *payload.DPOSProposalVote) (succeed bool, finished bool) {
	log.Info("[Normal-ProcessRejectVote] start")
	defer log.Info("[Normal-ProcessRejectVote] end")

	if !h.consensus.IsRunning() {
		log.Info("[Normal-ProcessRejectVote] consensus is not running")
		return false, false
	}

	currentProposal, ok := h.tryGetCurrentProposal(id, p)
	if !ok {
		h.proposalDispatcher.AddPendingVote(p)
	} else if currentProposal.IsEqual(p.ProposalHash) {
		return h.proposalDispatcher.ProcessVote(p, false)
	}

	return false, false
}

func (h *DPOSNormalHandler) tryGetCurrentProposal(id peer.PID, p *payload.DPOSProposalVote) (common.Uint256, bool) {
	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal == nil {
		requestProposal := &msg.RequestProposal{ProposalHash: p.ProposalHash}
		h.cfg.Network.SendMessageToPeer(id, requestProposal)
		return common.Uint256{}, false
	}
	return currentProposal.Hash(), true
}

func (h *DPOSNormalHandler) ProcessProposal(id peer.PID, p *payload.DPOSProposal) (handled bool) {
	log.Info("[Normal][ProcessProposal] start")
	defer log.Info("[Normal][ProcessProposal] end")

	if h.consensus.IsRunning() {
		h.consensus.TryChangeView()
	}

	needRecord, handled := h.proposalDispatcher.ProcessProposal(id, p, false)
	if needRecord {
		h.proposalDispatcher.illegalMonitor.AddProposal(p)
	}
	return handled
}

func (h *DPOSNormalHandler) ChangeView(firstBlockHash *common.Uint256) {
	log.Info("[OnViewChanged] clean proposal")
	h.proposalDispatcher.CleanProposals(true)
}

func (h *DPOSNormalHandler) TryStartNewConsensus(b *types.Block) bool {
	result := false

	if h.consensus.IsReady() {
		log.Info("[Normal][OnBlockReceived] received first unsigned block, start consensus")
		h.consensus.StartConsensus(b)
		result = true
	} else { //running
		log.Info("[Normal][OnBlockReceived] received unsigned block, record block")
		h.consensus.ProcessBlock(b)
	}

	return result
}
