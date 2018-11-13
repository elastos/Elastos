package handler

import (
	"bytes"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/core"
	common2 "github.com/elastos/Elastos.ELA/dpos/arbitration/common"
	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *cs.ConsensusStatus) error
	RecoverFromConsensusStatus(status *cs.ConsensusStatus) error
}

type IProposalDispatcher interface {
	AbnormalRecovering

	Initialize(consensus IConsensus, eventMonitor *log.EventMonitor)

	//status
	GetProcessingBlock() *core.Block
	IsVoteSlotEmpty() bool
	CurrentHeight() uint32

	//proposal
	StartProposal(b *core.Block)
	CleanProposals()
	FinishProposal()
	TryStartSpeculatingProposal(b *core.Block)
	ProcessProposal(d common2.DPosProposal)

	FinishConsensus()

	ProcessVote(v common2.DPosProposalVote, accept bool)

	RequestAbnormalRecovering()
	TryAppendAndBroadcastConfirmBlockMsg() bool
}

type IConsensus interface {
	AbnormalRecovering

	IsRunning() bool
	SetRunning()
	IsReady() bool
	SetReady()

	IsOnDuty() bool
	SetOnDuty(onDuty bool)

	RunWithStatusCondition(condition bool, closure func())
	RunWithAllStatusConditions(ready func(), running func())

	IsArbitratorOnDuty(arbitrator string) bool
	GetOnDutyArbitrator() string

	StartConsensus(b *core.Block)
	ProcessBlock(b *core.Block)

	ChangeView()
	TryChangeView() bool
	GetViewOffset() uint32

	StartHeartHeat()
	ResponseHeartBeat(peer *peer.Peer, cmd string, height uint32)
	ResetHeartBeatInterval(arbitrator string)
}

type DposHandlerSwitch struct {
	proposalDispatcher IProposalDispatcher
	consensus          IConsensus

	onDutyHandler  *DposOnDutyHandler
	normalHandler  *DposNormalHandler
	currentHandler DposEventConditionHandler

	locker       sync.Locker
	eventMonitor *log.EventMonitor

	isAbnormal bool
}

func (h *DposHandlerSwitch) Initialize(dispatcher IProposalDispatcher, consensus IConsensus, locker sync.Locker) {
	h.proposalDispatcher = dispatcher
	h.consensus = consensus
	h.locker = locker
	h.isAbnormal = false

	h.normalHandler = &DposNormalHandler{h}
	h.onDutyHandler = &DposOnDutyHandler{h}

	h.eventMonitor = &log.EventMonitor{}
	h.eventMonitor.Initialize()

	eventRecorder := &store.EventRecord{}
	eventRecorder.Initialize()
	eventLogs := &log.EventLogs{}

	h.eventMonitor.RegisterListener(eventRecorder)
	h.eventMonitor.RegisterListener(eventLogs)
	h.proposalDispatcher.Initialize(consensus, h.eventMonitor)
	h.SwitchTo(false)
}

func (h *DposHandlerSwitch) AddListeners(listeners ...log.EventListener) {
	for _, l := range listeners {
		h.eventMonitor.RegisterListener(l)
	}
}

func (h *DposHandlerSwitch) SwitchTo(onDuty bool) {
	if onDuty {
		h.currentHandler = h.onDutyHandler
	} else {
		h.currentHandler = h.normalHandler
	}
	h.consensus.SetOnDuty(true)
}

func (h *DposHandlerSwitch) FinishConsensus() {

	h.consensus.RunWithStatusCondition(true, func() {
		h.proposalDispatcher.FinishConsensus()
	})
}

func (h *DposHandlerSwitch) StartNewProposal(p common2.DPosProposal) {
	h.currentHandler.StartNewProposal(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	proposalEvent := log.ProposalEvent{
		Proposal:     p.Sponsor,
		BlockHash:    p.BlockHash,
		ReceivedTime: time.Now(),
		RawData:      rawData.Bytes(),
		Result:       false,
	}
	h.eventMonitor.OnProposalArrived(proposalEvent)
}

func (h *DposHandlerSwitch) ChangeView(firstBlockHash *common.Uint256) {
	h.currentHandler.ChangeView(firstBlockHash)

	viewEvent := log.ViewEvent{
		OnDutyArbitrator: h.consensus.GetOnDutyArbitrator(),
		StartTime:        time.Now(),
		Offset:           h.consensus.GetViewOffset(),
		Height:           h.proposalDispatcher.CurrentHeight(),
	}
	h.eventMonitor.OnViewStarted(viewEvent)
}

func (h *DposHandlerSwitch) TryStartNewConsensus(peer *peer.Peer, b *core.Block) bool {
	if _, ok := ArbitratorSingleton.BlockCache.TryGetValue(b.Hash()); ok {
		log.Info("[TryStartNewConsensus] failed, already have the block")
		return false
	}

	if h.proposalDispatcher.IsVoteSlotEmpty() {
		if h.currentHandler.TryStartNewConsensus(peer, b) {
			rawData := new(bytes.Buffer)
			b.Serialize(rawData)
			c := log.ConsensusEvent{StartTime: time.Now(), Height: b.Height, RawData: rawData.Bytes()}
			h.eventMonitor.OnConsensusStarted(c)
			return true
		}
	}

	//todo record block into database
	return false
}

func (h *DposHandlerSwitch) ProcessAcceptVote(p common2.DPosProposalVote) {
	h.currentHandler.ProcessAcceptVote(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: p.Signer, ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(voteEvent)
}

func (h *DposHandlerSwitch) ProcessRejectVote(p common2.DPosProposalVote) {
	h.currentHandler.ProcessRejectVote(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: p.Signer, ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(voteEvent)
}

func (h *DposHandlerSwitch) ResponseGetBlocks(peer *peer.Peer, startBlockHeight, endBlockHeight uint32) {
	//todo limit max height range (endBlockHeight - startBlockHeight)
	currentHeight := h.proposalDispatcher.CurrentHeight()

	endHeight := endBlockHeight
	if currentHeight < endBlockHeight {
		endHeight = currentHeight
	}
	blocks, blockConfirms, err := ArbitratorSingleton.Leger.GetBlocksAndConfirms(startBlockHeight, endHeight)
	if err != nil {
		log.Error(err)
		return
	}

	if currentBlock := h.proposalDispatcher.GetProcessingBlock(); currentBlock != nil {
		blocks = append(blocks, currentBlock)
	}

	msg := &cs.ResponseBlocksMessage{Command: cs.ResponseBlocks, Blocks: blocks, BlockConfirms: blockConfirms}
	peer.SendMessage(msg, nil)
}

func (h *DposHandlerSwitch) ProcessPing(peer *peer.Peer, height uint32) {
	h.processHeartBeat(peer, height, true)
}

func (h *DposHandlerSwitch) ProcessPong(peer *peer.Peer, height uint32) {
	h.processHeartBeat(peer, height, false)
}

func (h *DposHandlerSwitch) RequestAbnormalRecovering() {
	h.proposalDispatcher.RequestAbnormalRecovering()
	h.isAbnormal = true
}

func (h *DposHandlerSwitch) HelpToRecoverAbnormal(peer *peer.Peer, height uint32) {
	status := &cs.ConsensusStatus{}
	result := false

	h.consensus.RunWithStatusCondition(true, func() {
		if err := ArbitratorSingleton.Leger.CollectConsensusStatus(height, status.MissingBlocks, status.MissingBlockConfirms); err != nil {
			log.Error("Error occurred when collect consensus status from leger: ", err)
			return
		}

		if err := h.consensus.CollectConsensusStatus(height, status); err != nil {
			log.Error("Error occurred when collect consensus status from consensus object: ", err)
			return
		}

		if err := h.proposalDispatcher.CollectConsensusStatus(height, status); err != nil {
			log.Error("Error occurred when collect consensus status from proposal dispatcher object: ", err)
			return
		}

		result = true
	})

	if result {
		msg := &cs.ResponseConsensusMessage{Command: cs.ResponseConsensus, Consensus: *status}
		peer.SendMessage(msg, nil)
	}
}

func (h *DposHandlerSwitch) RecoverAbnormal(status *cs.ConsensusStatus) {
	ArbitratorSingleton.Leger.Restore()
	result := false

	h.consensus.RunWithStatusCondition(true, func() {
		if err := ArbitratorSingleton.Leger.RecoverFromConsensusStatus(status.MissingBlocks, status.MissingBlockConfirms); err != nil {
			log.Error("Error occurred when recover leger: ", err)
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

		result = true
	})

	if result {
		h.isAbnormal = false
	} else {
		ArbitratorSingleton.Leger.Rollback()
	}
}

func (h *DposHandlerSwitch) OnViewChanged(isOnDuty bool) {
	h.SwitchTo(isOnDuty)

	firstBlockHash, ok := ArbitratorSingleton.BlockCache.GetFirstArrivedBlockHash()
	if isOnDuty && !ok {
		log.Warn("[OnViewChanged] firstBlockHash is nil")
		return
	}
	log.Info("OnViewChanged, onduty, getBlock from first block hash:", firstBlockHash)
	h.ChangeView(&firstBlockHash)
}

func (h *DposHandlerSwitch) processHeartBeat(peer *peer.Peer, height uint32, needResponse bool) {
	currentHeight := h.proposalDispatcher.CurrentHeight()
	name, ok := GetPeerName(peer)

	if h.tryRequestBlocks(peer, height) {
		log.Info("Found higher block, requesting it.")
	}

	if needResponse {
		h.consensus.ResponseHeartBeat(peer, cs.Pong, currentHeight)
	}
	if ok {
		h.consensus.ResetHeartBeatInterval(name)
	}
}

func (h *DposHandlerSwitch) tryRequestBlocks(peer *peer.Peer, sourceHeight uint32) bool {
	height := h.proposalDispatcher.CurrentHeight()
	if sourceHeight > height {
		msg := &cs.GetBlocksMessage{Command: cs.GetBlocks,
			StartBlockHeight: height,
			EndBlockHeight:   sourceHeight}
		peer.SendMessage(msg, nil)

		return true
	}
	return false
}

func (h *DposHandlerSwitch) ChangeViewLoop() {
	for {
		h.locker.Lock()
		if h.consensus.IsRunning() {
			h.consensus.TryChangeView()
		}
		h.locker.Unlock()

		time.Sleep(1 * time.Second)
	}
}
