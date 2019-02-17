package manager

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/log"
	msg2 "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA/common"
)

type DposEventConditionHandler interface {
	TryStartNewConsensus(b *types.Block) bool

	ChangeView(firstBlockHash *common.Uint256)

	StartNewProposal(p types.DPosProposal)

	ProcessAcceptVote(id peer.PID, p types.DPosProposalVote)
	ProcessRejectVote(id peer.PID, p types.DPosProposalVote)
}

type DposHandlerSwitch interface {
	ViewListener
	DposEventConditionHandler

	Initialize(dispatcher *ProposalDispatcher, consensus *Consensus)
	SwitchTo(isOnDuty bool)

	FinishConsensus()

	RequestAbnormalRecovering()
	HelpToRecoverAbnormal(id peer.PID, height uint32)
	RecoverAbnormal(status *msg2.ConsensusStatus)

	ResponseGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32)
}

type dposHandlerSwitch struct {
	proposalDispatcher *ProposalDispatcher
	consensus          *Consensus
	network            DposNetwork
	manager            *DPOSManager
	arbitrators        interfaces.Arbitrators

	onDutyHandler  *DposOnDutyHandler
	normalHandler  *DposNormalHandler
	currentHandler DposEventConditionHandler

	eventMonitor *log.EventMonitor

	isAbnormal bool
}

func NewHandler(network DposNetwork, manager *DPOSManager,
	monitor *log.EventMonitor, arbitrators interfaces.Arbitrators) DposHandlerSwitch {
	h := &dposHandlerSwitch{
		network:      network,
		manager:      manager,
		eventMonitor: monitor,
		arbitrators:  arbitrators,
		isAbnormal:   false,
	}

	h.normalHandler = &DposNormalHandler{h}
	h.onDutyHandler = &DposOnDutyHandler{h}

	return h
}

func (h *dposHandlerSwitch) Initialize(dispatcher *ProposalDispatcher,
	consensus *Consensus) {
	h.proposalDispatcher = dispatcher
	h.consensus = consensus
	currentArbiter := h.manager.GetArbitrators().GetNextOnDutyArbitrator(h.consensus.GetViewOffset())
	isDposOnDuty := common.BytesToHexString(currentArbiter) == config.Parameters.ArbiterConfiguration.PublicKey
	h.SwitchTo(isDposOnDuty)
}

func (h *dposHandlerSwitch) AddListeners(listeners ...log.EventListener) {
	for _, l := range listeners {
		h.eventMonitor.RegisterListener(l)
	}
}

func (h *dposHandlerSwitch) SwitchTo(onDuty bool) {
	if onDuty {
		h.currentHandler = h.onDutyHandler
	} else {
		h.currentHandler = h.normalHandler
	}
	h.consensus.SetOnDuty(onDuty)
}

func (h *dposHandlerSwitch) FinishConsensus() {
	h.proposalDispatcher.FinishConsensus()
}

func (h *dposHandlerSwitch) StartNewProposal(p types.DPosProposal) {
	h.currentHandler.StartNewProposal(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	proposalEvent := log.ProposalEvent{
		Proposal:     common.BytesToHexString(p.Sponsor),
		BlockHash:    p.BlockHash,
		ReceivedTime: time.Now(),
		ProposalHash: p.Hash(),
		RawData:      rawData.Bytes(),
		Result:       false,
	}
	h.eventMonitor.OnProposalArrived(&proposalEvent)
}

func (h *dposHandlerSwitch) ChangeView(firstBlockHash *common.Uint256) {
	h.currentHandler.ChangeView(firstBlockHash)

	viewEvent := log.ViewEvent{
		OnDutyArbitrator: common.BytesToHexString(h.consensus.GetOnDutyArbitrator()),
		StartTime:        time.Now(),
		Offset:           h.consensus.GetViewOffset(),
		Height:           h.proposalDispatcher.CurrentHeight(),
	}
	h.eventMonitor.OnViewStarted(&viewEvent)
}

func (h *dposHandlerSwitch) TryStartNewConsensus(b *types.Block) bool {
	if _, ok := h.manager.GetBlockCache().TryGetValue(b.Hash()); ok {
		log.Info("[TryStartNewConsensus] failed, already have the block")
		return false
	}

	if h.proposalDispatcher.IsProcessingBlockEmpty() {
		if h.currentHandler.TryStartNewConsensus(b) {
			rawData := new(bytes.Buffer)
			b.Serialize(rawData)
			c := log.ConsensusEvent{StartTime: time.Now(), Height: b.Height, RawData: rawData.Bytes()}
			h.eventMonitor.OnConsensusStarted(&c)
			return true
		}
	}

	//todo record block into database
	return false
}

func (h *dposHandlerSwitch) ProcessAcceptVote(id peer.PID, p types.DPosProposalVote) {
	h.currentHandler.ProcessAcceptVote(id, p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(p.Signer), ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(&voteEvent)
}

func (h *dposHandlerSwitch) ProcessRejectVote(id peer.PID, p types.DPosProposalVote) {
	h.currentHandler.ProcessRejectVote(id, p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(p.Signer), ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(&voteEvent)
}

func (h *dposHandlerSwitch) ResponseGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32) {
	//todo limit max height range (endBlockHeight - startBlockHeight)
	currentHeight := h.proposalDispatcher.CurrentHeight()

	endHeight := endBlockHeight
	if currentHeight < endBlockHeight {
		endHeight = currentHeight
	}
	blockConfirms, err := blockchain.DefaultLedger.GetDposBlocks(startBlockHeight, endHeight)
	if err != nil {
		log.Error(err)
		return
	}

	if currentBlock := h.proposalDispatcher.GetProcessingBlock(); currentBlock != nil {
		blockConfirms = append(blockConfirms, &types.DposBlock{
			BlockFlag: true,
			Block:     currentBlock,
		})
	}

	msg := &msg2.ResponseBlocks{Command: msg2.CmdResponseBlocks, BlockConfirms: blockConfirms}
	h.network.SendMessageToPeer(id, msg)
}

func (h *dposHandlerSwitch) RequestAbnormalRecovering() {
	h.proposalDispatcher.RequestAbnormalRecovering()
	h.isAbnormal = true
}

func (h *dposHandlerSwitch) HelpToRecoverAbnormal(id peer.PID, height uint32) {
	status := &msg2.ConsensusStatus{}
	log.Info("[HelpToRecoverAbnormal] peer id:", common.BytesToHexString(id[:]))

	if err := h.consensus.CollectConsensusStatus(height, status); err != nil {
		log.Error("Error occurred when collect consensus status from consensus object: ", err)
		return
	}

	if err := h.proposalDispatcher.CollectConsensusStatus(height, status); err != nil {
		log.Error("Error occurred when collect consensus status from proposal dispatcher object: ", err)
		return
	}

	msg := &msg2.ResponseConsensus{Consensus: *status}
	h.network.SendMessageToPeer(id, msg)
}

func (h *dposHandlerSwitch) RecoverAbnormal(status *msg2.ConsensusStatus) {
	if !h.isAbnormal {
		return
	}

	if err := h.proposalDispatcher.RecoverFromConsensusStatus(status); err != nil {
		log.Error("Error occurred when recover proposal dispatcher object: ", err)
		return
	}

	if err := h.consensus.RecoverFromConsensusStatus(status); err != nil {
		log.Error("Error occurred when recover consensus object: ", err)
		return
	}

	h.isAbnormal = false
}

func (h *dposHandlerSwitch) OnViewChanged(isOnDuty bool) {
	h.SwitchTo(isOnDuty)

	firstBlockHash, ok := h.manager.GetBlockCache().GetFirstArrivedBlockHash()
	if isOnDuty && !ok {
		log.Warn("[OnViewChanged] firstBlockHash is nil")
		return
	}
	log.Info("OnViewChanged, onduty, getBlock from first block hash:", firstBlockHash)
	h.ChangeView(&firstBlockHash)
}
