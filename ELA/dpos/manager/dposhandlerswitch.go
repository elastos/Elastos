package manager

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/dpos/log"
	msg2 "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
)

type DPOSEventConditionHandler interface {
	TryStartNewConsensus(b *types.Block) bool

	ChangeView(firstBlockHash *common.Uint256)

	StartNewProposal(p payload.DPOSProposal)

	ProcessAcceptVote(id peer.PID, p payload.DPOSProposalVote) (succeed bool, finished bool)
	ProcessRejectVote(id peer.PID, p payload.DPOSProposalVote) (succeed bool, finished bool)
}

type DPOSHandlerConfig struct {
	Network     DPOSNetwork
	Manager     *DPOSManager
	Monitor     *log.EventMonitor
	Arbitrators interfaces.Arbitrators
}

type DPOSHandlerSwitch struct {
	proposalDispatcher *ProposalDispatcher
	consensus          *Consensus
	cfg                DPOSHandlerConfig

	onDutyHandler  *DPOSOnDutyHandler
	normalHandler  *DPOSNormalHandler
	currentHandler DPOSEventConditionHandler

	isAbnormal bool
}

func NewHandler(cfg DPOSHandlerConfig) *DPOSHandlerSwitch {

	h := &DPOSHandlerSwitch{
		cfg:        cfg,
		isAbnormal: false,
	}

	h.normalHandler = &DPOSNormalHandler{h}
	h.onDutyHandler = &DPOSOnDutyHandler{h}

	return h
}

func (h *DPOSHandlerSwitch) Initialize(dispatcher *ProposalDispatcher,
	consensus *Consensus) {
	h.proposalDispatcher = dispatcher
	h.consensus = consensus
	currentArbiter := h.cfg.Manager.GetArbitrators().GetNextOnDutyArbitrator(h.
		consensus.GetViewOffset())
	isDposOnDuty := common.BytesToHexString(currentArbiter) == config.
		Parameters.ArbiterConfiguration.PublicKey
	h.SwitchTo(isDposOnDuty)
}

func (h *DPOSHandlerSwitch) AddListeners(listeners ...log.EventListener) {
	for _, l := range listeners {
		h.cfg.Monitor.RegisterListener(l)
	}
}

func (h *DPOSHandlerSwitch) SwitchTo(onDuty bool) {
	if onDuty {
		h.currentHandler = h.onDutyHandler
	} else {
		h.currentHandler = h.normalHandler
	}
	h.consensus.SetOnDuty(onDuty)
}

func (h *DPOSHandlerSwitch) FinishConsensus() {
	h.proposalDispatcher.FinishConsensus()
}

func (h *DPOSHandlerSwitch) StartNewProposal(p payload.DPOSProposal) {
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
	h.cfg.Monitor.OnProposalArrived(&proposalEvent)
}

func (h *DPOSHandlerSwitch) ChangeView(firstBlockHash *common.Uint256) {
	h.currentHandler.ChangeView(firstBlockHash)

	viewEvent := log.ViewEvent{
		OnDutyArbitrator: common.BytesToHexString(h.consensus.GetOnDutyArbitrator()),
		StartTime:        time.Now(),
		Offset:           h.consensus.GetViewOffset(),
		Height:           h.proposalDispatcher.CurrentHeight(),
	}
	h.cfg.Monitor.OnViewStarted(&viewEvent)
}

func (h *DPOSHandlerSwitch) TryStartNewConsensus(b *types.Block) bool {
	if _, ok := h.cfg.Manager.GetBlockCache().TryGetValue(b.Hash()); ok {
		log.Info("[TryStartNewConsensus] failed, already have the block")
		return false
	}

	if h.proposalDispatcher.IsProcessingBlockEmpty() {
		if h.currentHandler.TryStartNewConsensus(b) {
			rawData := new(bytes.Buffer)
			b.Header.Serialize(rawData)
			c := log.ConsensusEvent{StartTime: time.Now(), Height: b.Height, RawData: rawData.Bytes()}
			h.cfg.Monitor.OnConsensusStarted(&c)
			return true
		}
	}

	//todo record block into database
	return false
}

func (h *DPOSHandlerSwitch) ProcessAcceptVote(id peer.PID, p payload.DPOSProposalVote) (bool, bool) {
	succeed, finished := h.currentHandler.ProcessAcceptVote(id, p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(p.Signer), ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	h.cfg.Monitor.OnVoteArrived(&voteEvent)

	return succeed, finished
}

func (h *DPOSHandlerSwitch) ProcessRejectVote(id peer.PID, p payload.DPOSProposalVote) (bool, bool) {
	succeed, finished := h.currentHandler.ProcessRejectVote(id, p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: common.BytesToHexString(p.Signer), ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	h.cfg.Monitor.OnVoteArrived(&voteEvent)

	return succeed, finished
}

func (h *DPOSHandlerSwitch) ResponseGetBlocks(id peer.PID, startBlockHeight, endBlockHeight uint32) {
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
	h.cfg.Network.SendMessageToPeer(id, msg)
}

func (h *DPOSHandlerSwitch) RequestAbnormalRecovering() {
	h.proposalDispatcher.RequestAbnormalRecovering()
	h.isAbnormal = true
}

func (h *DPOSHandlerSwitch) HelpToRecoverAbnormal(id peer.PID, height uint32) {
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
	h.cfg.Network.SendMessageToPeer(id, msg)
}

func (h *DPOSHandlerSwitch) RecoverAbnormal(status *msg2.ConsensusStatus) {
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

func (h *DPOSHandlerSwitch) OnViewChanged(isOnDuty bool) {
	h.SwitchTo(isOnDuty)

	firstBlockHash, ok := h.cfg.Manager.GetBlockCache().GetFirstArrivedBlockHash()
	if isOnDuty && !ok {
		log.Warn("[OnViewChanged] firstBlockHash is nil")
		return
	}
	log.Info("OnViewChanged, onduty, getBlock from first block hash:", firstBlockHash)
	h.ChangeView(&firstBlockHash)
}
