package handler

import (
	"bytes"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/manager"
	"github.com/elastos/Elastos.ELA/dpos/log"
	msg2 "github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/dpos/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type DposHandlerSwitch struct {
	proposalDispatcher IProposalDispatcher
	consensus          IConsensus
	network            DposNetwork

	onDutyHandler  *DposOnDutyHandler
	normalHandler  *DposNormalHandler
	currentHandler DposEventConditionHandler

	locker       sync.Locker
	eventMonitor *log.EventMonitor

	isAbnormal bool
}

func (h *DposHandlerSwitch) Initialize(dispatcher IProposalDispatcher, consensus IConsensus, network DposNetwork, locker sync.Locker) {
	h.proposalDispatcher = dispatcher
	h.consensus = consensus
	h.network = network
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
	h.proposalDispatcher.Initialize(consensus, h.eventMonitor, network)
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

func (h *DposHandlerSwitch) StartNewProposal(p core.DPosProposal) {
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

func (h *DposHandlerSwitch) TryStartNewConsensus(b *core.Block) bool {
	if _, ok := ArbitratorSingleton.BlockCache.TryGetValue(b.Hash()); ok {
		log.Info("[TryStartNewConsensus] failed, already have the block")
		return false
	}

	if h.proposalDispatcher.IsVoteSlotEmpty() {
		if h.currentHandler.TryStartNewConsensus(b) {
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

func (h *DposHandlerSwitch) ProcessAcceptVote(p core.DPosProposalVote) {
	h.currentHandler.ProcessAcceptVote(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: p.Signer, ReceivedTime: time.Now(), Result: true, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(voteEvent)
}

func (h *DposHandlerSwitch) ProcessRejectVote(p core.DPosProposalVote) {
	h.currentHandler.ProcessRejectVote(p)

	rawData := new(bytes.Buffer)
	p.Serialize(rawData)
	voteEvent := log.VoteEvent{Signer: p.Signer, ReceivedTime: time.Now(), Result: false, RawData: rawData.Bytes()}
	h.eventMonitor.OnVoteArrived(voteEvent)
}

func (h *DposHandlerSwitch) ResponseGetBlocks(id common.Uint256, startBlockHeight, endBlockHeight uint32) {
	//todo limit max height range (endBlockHeight - startBlockHeight)
	currentHeight := h.proposalDispatcher.CurrentHeight()

	endHeight := endBlockHeight
	if currentHeight < endBlockHeight {
		endHeight = currentHeight
	}
	blocks, blockConfirms, err := blockchain.DefaultLedger.GetBlocksAndConfirms(startBlockHeight, endHeight)
	if err != nil {
		log.Error(err)
		return
	}

	if currentBlock := h.proposalDispatcher.GetProcessingBlock(); currentBlock != nil {
		blocks = append(blocks, currentBlock)
	}

	msg := &msg2.ResponseBlocksMessage{Command: msg2.ResponseBlocks, Blocks: blocks, BlockConfirms: blockConfirms}
	h.network.SendMessageToPeer(id, msg)
}

func (h *DposHandlerSwitch) ProcessPing(id common.Uint256, height uint32) {
	h.processHeartBeat(id, height)
}

func (h *DposHandlerSwitch) ProcessPong(id common.Uint256, height uint32) {
	h.processHeartBeat(id, height)
}

func (h *DposHandlerSwitch) RequestAbnormalRecovering() {
	h.proposalDispatcher.RequestAbnormalRecovering()
	h.isAbnormal = true
}

func (h *DposHandlerSwitch) HelpToRecoverAbnormal(id common.Uint256, height uint32) {
	status := &msg2.ConsensusStatus{}
	result := false

	h.consensus.RunWithStatusCondition(true, func() {
		var err error
		if status.MissingBlocks, status.MissingBlockConfirms, err = blockchain.DefaultLedger.GetBlocksAndConfirms(height, 0); err != nil {
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
		msg := &msg2.ResponseConsensusMessage{Consensus: *status}
		h.network.SendMessageToPeer(id, msg)
	}
}

func (h *DposHandlerSwitch) RecoverAbnormal(status *msg2.ConsensusStatus) {
	result := false

	h.consensus.RunWithStatusCondition(true, func() {
		if err := blockchain.DefaultLedger.AppendBlocksAndConfirms(status.MissingBlocks, status.MissingBlockConfirms); err != nil {
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

func (h *DposHandlerSwitch) processHeartBeat(id common.Uint256, height uint32) {
	if h.tryRequestBlocks(id, height) {
		log.Info("Found higher block, requesting it.")
	}
}

func (h *DposHandlerSwitch) tryRequestBlocks(id common.Uint256, sourceHeight uint32) bool {
	height := h.proposalDispatcher.CurrentHeight()
	if sourceHeight > height {
		msg := &msg2.GetBlocksMessage{
			StartBlockHeight: height,
			EndBlockHeight:   sourceHeight}
		h.network.SendMessageToPeer(id, msg)

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
