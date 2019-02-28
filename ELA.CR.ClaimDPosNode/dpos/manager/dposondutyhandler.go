package manager

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type DPOSOnDutyHandler struct {
	*DPOSHandlerSwitch
}

func (h *DPOSOnDutyHandler) ProcessAcceptVote(id peer.PID, p payload.DPOSProposalVote) {
	log.Info("[Onduty-ProcessAcceptVote] start")
	defer log.Info("[Onduty-ProcessAcceptVote] end")

	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal != nil && currentProposal.Hash().IsEqual(p.ProposalHash) && h.consensus.IsRunning() {
		log.Info("[OnVoteReceived] Received needed sign, collect it")
		h.proposalDispatcher.ProcessVote(p, true)
	}
}

func (h *DPOSOnDutyHandler) ProcessRejectVote(id peer.PID, p payload.DPOSProposalVote) {
	log.Info("[Onduty-ProcessRejectVote] start")

	currentProposal := h.proposalDispatcher.GetProcessingProposal()
	if currentProposal != nil && currentProposal.Hash().IsEqual(p.ProposalHash) && h.consensus.IsRunning() {
		h.proposalDispatcher.ProcessVote(p, false)
	}
}

func (h *DPOSOnDutyHandler) StartNewProposal(p payload.DPOSProposal) {
}

func (h *DPOSOnDutyHandler) ChangeView(firstBlockHash *common.Uint256) {

	if !h.tryCreateInactiveArbitratorsTx() {
		b, ok := h.cfg.Manager.GetBlockCache().TryGetValue(*firstBlockHash)
		if !ok {
			log.Info("[OnViewChanged] get block failed for proposal")
		} else {
			log.Info("[OnViewChanged] start proposal")
			h.proposalDispatcher.CleanProposals(true)
			h.proposalDispatcher.StartProposal(b)
		}
	}
}

func (h *DPOSOnDutyHandler) TryStartNewConsensus(b *types.Block) bool {
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

func (h *DPOSOnDutyHandler) tryCreateInactiveArbitratorsTx() bool {
	if h.proposalDispatcher.IsViewChangedTimeOut() {
		tx, err := h.proposalDispatcher.CreateInactiveArbitrators()
		if err != nil {
			log.Warn("[tryCreateInactiveArbitratorsTx] create tx error: ", err)
			return false
		}

		h.cfg.Network.BroadcastMessage(&msg.Tx{Serializable: tx})
		return true
	}
	return false
}
