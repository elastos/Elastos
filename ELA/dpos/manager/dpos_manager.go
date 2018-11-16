package manager

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"

	"github.com/elastos/Elastos.ELA.Utility/common"
	utip2p "github.com/elastos/Elastos.ELA.Utility/p2p"
)

type DposNetwork interface {
	Start()
	Stop() error

	SendMessageToPeer(id common.Uint256, msg utip2p.Message) error
	BroadcastMessage(msg utip2p.Message)

	Reset(epochInfo interface{}) error
	ChangeHeight(height uint32) error

	GetActivePeer() *common.Uint256
}

type StatusSyncEventListener interface {
	OnPing(id common.Uint256, height uint32)
	OnPong(id common.Uint256, height uint32)
	OnGetBlocks(id common.Uint256, startBlockHeight, endBlockHeight uint32)
	OnResponseBlocks(id common.Uint256, blocks []*core.Block, blockConfirms []*core.DPosProposalVoteSlot)
	OnRequestConsensus(id common.Uint256, height uint32)
	OnResponseConsensus(id common.Uint256, status *msg.ConsensusStatus)
}

type NetworkEventListener interface {
	StatusSyncEventListener

	OnProposalReceived(id common.Uint256, p core.DPosProposal)
	OnVoteReceived(id common.Uint256, p core.DPosProposalVote)
	OnVoteRejected(id common.Uint256, p core.DPosProposalVote)

	OnChangeView()
	OnBadNetwork()

	OnBlockReceived(b *core.Block, confirmed bool)
	OnConfirmReceived(p *core.DPosProposalVoteSlot)
}

type AbnormalRecovering interface {
	CollectConsensusStatus(height uint32, status *msg.ConsensusStatus) error
	RecoverFromConsensusStatus(status *msg.ConsensusStatus) error
}

type DposManager interface {
	NetworkEventListener

	GetPublicKey() string
	GetBlockCache() *ConsensusBlockCache

	Initialize(handler DposHandlerSwitch, dispatcher ProposalDispatcher, consensus Consensus, network DposNetwork)

	Recover()

	ProcessHigherBlock(b *core.Block)
	ConfirmBlock()

	ChangeConsensus(onDuty bool)
}

type dposManager struct {
	publicKey  string
	blockCache *ConsensusBlockCache

	handler    DposHandlerSwitch
	network    DposNetwork
	dispatcher ProposalDispatcher
	consensus  Consensus
}

func NewManager(name string, ) DposManager {
	m := &dposManager{
		publicKey:  name,
		blockCache: &ConsensusBlockCache{},
	}
	m.blockCache.Reset()

	return m
}

func (d *dposManager) Initialize(handler DposHandlerSwitch, dispatcher ProposalDispatcher, consensus Consensus, network DposNetwork) {
	d.handler = handler
	d.dispatcher = dispatcher
	d.consensus = consensus
	d.network = network
}

func (d *dposManager) GetPublicKey() string {
	return d.publicKey
}

func (d *dposManager) GetBlockCache() *ConsensusBlockCache {
	return d.blockCache
}

func (d *dposManager) Recover() {
	d.handler.RequestAbnormalRecovering()
}

func (d *dposManager) ProcessHigherBlock(b *core.Block) {
	d.handler.TryStartNewConsensus(b)
}

func (d *dposManager) ConfirmBlock() {
	d.handler.FinishConsensus()
}

func (d *dposManager) ChangeConsensus(onDuty bool) {
	d.handler.SwitchTo(onDuty)
}

func (d *dposManager) OnProposalReceived(id common.Uint256, p core.DPosProposal) {
	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")
	d.handler.StartNewProposal(p)
}

func (d *dposManager) OnVoteReceived(id common.Uint256, p core.DPosProposalVote) {
	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	d.handler.ProcessAcceptVote(p)
}

func (d *dposManager) OnVoteRejected(id common.Uint256, p core.DPosProposalVote) {
	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	d.handler.ProcessRejectVote(p)
}

func (d *dposManager) OnPing(id common.Uint256, height uint32) {
	d.handler.ProcessPing(id, height)
}

func (d *dposManager) OnPong(id common.Uint256, height uint32) {
	d.handler.ProcessPong(id, height)
}

func (d *dposManager) OnGetBlocks(id common.Uint256, startBlockHeight, endBlockHeight uint32) {
	d.handler.ResponseGetBlocks(id, startBlockHeight, endBlockHeight)
}

func (d *dposManager) OnResponseBlocks(id common.Uint256, blocks []*core.Block, blockConfirms []*core.DPosProposalVoteSlot) {
	log.Info("[OnResponseBlocks] start")
	defer log.Info("[OnResponseBlocks] end")

	if err := blockchain.DefaultLedger.AppendBlocksAndConfirms(blocks, blockConfirms); err != nil {
		log.Error("Response blocks error: ", err)
	}
}

func (d *dposManager) OnRequestConsensus(id common.Uint256, height uint32) {
	d.handler.HelpToRecoverAbnormal(id, height)
}

func (d *dposManager) OnResponseConsensus(id common.Uint256, status *msg.ConsensusStatus) {
	d.handler.RecoverAbnormal(status)
}

func (d *dposManager) OnBadNetwork() {
	d.dispatcher.OnAbnormalStateDetected()
}

func (d *dposManager) OnChangeView() {
	d.consensus.TryChangeView()
}

func (d *dposManager) OnBlockReceived(b *core.Block, confirmed bool) {
	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	if confirmed {
		d.ConfirmBlock()
		d.changeHeight()
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}

	if blockchain.DefaultLedger.Blockchain.BlockHeight < b.Height { //new height block coming
		d.ProcessHigherBlock(b)
	} else {
		log.Warn("a.Leger.LastBlock.Height", blockchain.DefaultLedger.Blockchain.BlockHeight, "b.Height", b.Height)
	}
}

func (d *dposManager) OnConfirmReceived(p *core.DPosProposalVoteSlot) {

	log.Info("[OnConfirmReceived] started, hash:", p.Hash)
	defer log.Info("[OnConfirmReceived] end")

	d.ConfirmBlock()
	d.changeHeight()
}

func (d *dposManager) changeHeight() {
	//fixme uncomment me later
	//if err := d.network.ChangeHeight(d.dispatcher.CurrentHeight()); err != nil{
	//	log.Error("Error occurred with change height: ", err)
	//	return
	//}

	currentArbiter, err := blockchain.GetOnDutyArbiter()
	if err != nil {
		log.Error("Error occurred with change height: get current arbiter error.")
		return
	}

	onDuty := d.publicKey == common.BytesToHexString(currentArbiter)

	if onDuty {
		log.Info("[onDutyArbitratorChanged] not onduty -> onduty")
	} else {
		log.Info("[onDutyArbitratorChanged] onduty -> not onduty")
	}
	d.ChangeConsensus(onDuty)
}
