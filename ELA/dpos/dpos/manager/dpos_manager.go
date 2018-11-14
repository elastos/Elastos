package manager

import (
	"sync"

	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/dpos/arbitration/cs"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	msg "github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

type DposEventConditionHandler interface {
	TryStartNewConsensus(b *core.Block) bool

	ChangeView(firstBlockHash *common.Uint256)

	StartNewProposal(p msg.DPosProposal)

	ProcessAcceptVote(p msg.DPosProposalVote)
	ProcessRejectVote(p msg.DPosProposalVote)
}

type DposEventHandler interface {
	DposEventConditionHandler

	SwitchTo(isOnDuty bool)

	FinishConsensus()

	ProcessPing(peer *peer.Peer, height uint32)
	ProcessPong(peer *peer.Peer, height uint32)

	RequestAbnormalRecovering()
	HelpToRecoverAbnormal(peer *peer.Peer, height uint32)
	RecoverAbnormal(status *cs.ConsensusStatus)

	ResponseGetBlocks(peer *peer.Peer, startBlockHeight, endBlockHeight uint32)
}

type DposManager struct {
	dposLock sync.Mutex
	handler  DposEventHandler
}

func (d *DposManager) Initialize(handler DposEventHandler) {
	d.handler = handler
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

func (d *DposManager) OnProposalReceived(peer *peer.Peer, p msg.DPosProposal) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnProposalReceived] started")
	defer log.Info("[OnProposalReceived] end")
	d.handler.StartNewProposal(p)
}

func (d *DposManager) OnVoteReceived(peer *peer.Peer, p msg.DPosProposalVote) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnVoteReceived] started")
	defer log.Info("[OnVoteReceived] end")
	d.handler.ProcessAcceptVote(p)
}

func (d *DposManager) OnVoteRejected(peer *peer.Peer, p msg.DPosProposalVote) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	log.Info("[OnVoteRejected] started")
	defer log.Info("[OnVoteRejected] end")
	d.handler.ProcessRejectVote(p)
}

func (d *DposManager) OnPing(peer *peer.Peer, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ProcessPing(peer, height)
}

func (d *DposManager) OnPong(peer *peer.Peer, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ProcessPong(peer, height)
}

func (d *DposManager) OnGetBlocks(peer *peer.Peer, startBlockHeight, endBlockHeight uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.ResponseGetBlocks(peer, startBlockHeight, endBlockHeight)
}

func (d *DposManager) OnResponseBlocks(peer *peer.Peer, blocks []*core.Block, blockConfirms []*msg.DPosProposalVoteSlot) {
	log.Info("[OnResponseBlocks] start")
	defer log.Info("[OnResponseBlocks] end")

	//todo call add blocks and confirms by ledger
	//for _, v := range blocks {
	//	ArbitratorSingleton.OnBlockReceived(peer, v)
	//}
	//for _, v := range blockConfirms {
	//	ArbitratorSingleton.OnConfirmReceived(peer, v)
	//}
}

func (d *DposManager) OnRequestConsensus(peer *peer.Peer, height uint32) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.HelpToRecoverAbnormal(peer, height)
}

func (d *DposManager) OnResponseConsensus(peer *peer.Peer, status *cs.ConsensusStatus) {
	d.dposLock.Lock()
	defer d.dposLock.Unlock()

	d.handler.RecoverAbnormal(status)
}
