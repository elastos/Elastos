package manager

import (
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
)

type DposEventConditionHandler interface {
	TryStartNewConsensus(b *core.Block) bool

	ChangeView(firstBlockHash *common.Uint256)

	StartNewProposal(p core.DPosProposal)

	ProcessAcceptVote(p core.DPosProposalVote)
	ProcessRejectVote(p core.DPosProposalVote)
}

type DposEventHandler interface {
	DposEventConditionHandler

	SwitchTo(isOnDuty bool)

	FinishConsensus()

	ProcessPing(id common.Uint256, height uint32)
	ProcessPong(id common.Uint256, height uint32)

	RequestAbnormalRecovering()
	HelpToRecoverAbnormal(id common.Uint256, height uint32)
	RecoverAbnormal(status *msg.ConsensusStatus)

	ResponseGetBlocks(id common.Uint256, startBlockHeight, endBlockHeight uint32)
}

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *msg.ConsensusStatus) error
	RecoverFromConsensusStatus(status *msg.ConsensusStatus) error
}

type IProposalDispatcher interface {
	AbnormalRecovering

	Initialize(consensus IConsensus, eventMonitor *log.EventMonitor, network arbitrator.DposNetwork)

	//status
	GetProcessingBlock() *core.Block
	IsVoteSlotEmpty() bool
	CurrentHeight() uint32

	//proposal
	StartProposal(b *core.Block)
	CleanProposals()
	FinishProposal()
	TryStartSpeculatingProposal(b *core.Block)
	ProcessProposal(d core.DPosProposal)

	FinishConsensus()

	ProcessVote(v core.DPosProposalVote, accept bool)

	OnAbnormalStateDetected()
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
}

type DposManager struct {
	dposLock   sync.Mutex
	handler    DposEventHandler
	network    arbitrator.DposNetwork
	dispatcher IProposalDispatcher
}

func (d *DposManager) Initialize(handler DposEventHandler, dispatcher IProposalDispatcher, network arbitrator.DposNetwork) {
	d.handler = handler
	d.network = network
	d.dispatcher = dispatcher
}

func (d *DposManager) Recover() {
	d.handler.RequestAbnormalRecovering()
}

func (d *DposManager) ProcessHigherBlock(b *core.Block) {
	d.handler.TryStartNewConsensus(b)
}

func (d *DposManager) ConfirmBlock() {
	d.handler.FinishConsensus()
}

func (d *DposManager) Lock() {
	d.dposLock.Lock()
}

func (d *DposManager) Unlock() {
	d.dposLock.Unlock()
}

func (d *DposManager) ChangeConsensus(onDuty bool) {
	d.handler.SwitchTo(onDuty)
}

func (d *DposManager) OnProposalReceived(id common.Uint256, p core.DPosProposal) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")
	d.handler.StartNewProposal(p)
}

func (d *DposManager) OnVoteReceived(id common.Uint256, p core.DPosProposalVote) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	d.handler.ProcessAcceptVote(p)
}

func (d *DposManager) OnVoteRejected(id common.Uint256, p core.DPosProposalVote) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	d.handler.ProcessRejectVote(p)
}

func (d *DposManager) OnPing(id common.Uint256, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ProcessPing(id, height)
}

func (d *DposManager) OnPong(id common.Uint256, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ProcessPong(id, height)
}

func (d *DposManager) OnGetBlocks(id common.Uint256, startBlockHeight, endBlockHeight uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ResponseGetBlocks(id, startBlockHeight, endBlockHeight)
}

func (d *DposManager) OnResponseBlocks(id common.Uint256, blocks []*core.Block, blockConfirms []*core.DPosProposalVoteSlot) {
	log.Info("[OnResponseBlocks] start")
	defer log.Info("[OnResponseBlocks] end")

	if err := blockchain.DefaultLedger.AppendBlocksAndConfirms(blocks, blockConfirms); err != nil {
		log.Error("Response blocks error: ", err)
	}
}

func (d *DposManager) OnRequestConsensus(id common.Uint256, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.HelpToRecoverAbnormal(id, height)
}

func (d *DposManager) OnResponseConsensus(id common.Uint256, status *msg.ConsensusStatus) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.RecoverAbnormal(status)
}

func (d *DposManager) OnBadNetwork() {
	d.dispatcher.OnAbnormalStateDetected()
}
