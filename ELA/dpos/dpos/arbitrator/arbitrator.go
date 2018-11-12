package arbitrator

import (
	"sync"

	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/chain"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/cache"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

var ArbitratorSingleton *Arbitrator

type IDposManager interface {
	sync.Locker

	Recover()

	ProcessHigherBlock(peer *peer.Peer, b *core.Block)
	ConfirmBlock()

	ChangeConsensus(onDuty bool)
}

type StatusRollback interface {
	Restore()
	Rollback() error
}

type Arbitrator struct {
	Name       string
	IsOnDuty   bool
	Leger      Ledger
	BlockCache ConsensusBlockCache

	DposManager IDposManager
}

func (a *Arbitrator) GetPublicKey() string {
	return a.Name
}

func (a *Arbitrator) OnDutyArbitratorChanged(onDuty bool) {
	if onDuty {
		log.Info("[OnDutyArbitratorChanged] not onduty -> onduty")
	} else {
		log.Info("[OnDutyArbitratorChanged] onduty -> not onduty")
	}
	a.IsOnDuty = onDuty
	a.DposManager.ChangeConsensus(onDuty)
}

func (a *Arbitrator) OnBlockReceived(peer *peer.Peer, b *core.Block) {
	a.DposManager.Lock()
	defer a.DposManager.Unlock()

	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	confirm, ok := ArbitratorSingleton.Leger.GetPendingConfirms(b.Hash())
	if ok && a.tryConfirmBlock(b, confirm) {
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}

	if a.Leger.LastBlock == nil || a.Leger.LastBlock.Height < b.Height { //new height block coming
		a.DposManager.ProcessHigherBlock(peer, b)
	} else {
		log.Warn("a.Leger.LastBlock.Height", a.Leger.LastBlock.Height, "b.Height", b.Height)
	}

}

func (a *Arbitrator) OnConfirmReceived(peer *peer.Peer, p *ProposalVoteSlot) {
	a.DposManager.Lock()
	defer a.DposManager.Unlock()

	log.Info("[OnConfirmReceived] started, hash:", p.Hash)
	defer log.Info("[OnConfirmReceived] end")

	block, ok := ArbitratorSingleton.BlockCache.TryGetValue(p.Hash)
	if !ok {
		ArbitratorSingleton.Leger.AppendPendingConfirms(p)
		log.Warn("[OnConfirmReceived] invalid block:", block)
		return
	}
	if a.Leger.LastBlock == nil || a.Leger.LastBlock.Height < block.Height {
		a.tryConfirmBlock(block, p)
	}
}

func (a *Arbitrator) tryConfirmBlock(b *core.Block, p *ProposalVoteSlot) bool {
	if a.Leger.TryAppendBlock(b, p) {
		log.Info("[TryAppendBlock] succeed")
		a.DposManager.ConfirmBlock()
		a.ChangeHeight()
		return true
	}
	return false
}

func (a *Arbitrator) ChangeHeight() { //called by leger later
	blockHeight := uint32(0)
	if a.Leger.LastBlock != nil {
		blockHeight = a.Leger.LastBlock.Height
	}
	ArbitratorGroupSingleton.ChangeHeight(blockHeight)
}
