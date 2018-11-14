package arbitrator

import (
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/cache"
	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var ArbitratorSingleton *Arbitrator

type IDposManager interface {
	sync.Locker

	Recover()

	ProcessHigherBlock(b *core.Block)
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

func (a *Arbitrator) OnBlockReceived(b *core.Block, confirmed bool) {
	a.DposManager.Lock()
	defer a.DposManager.Unlock()

	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	if confirmed {
		a.DposManager.ConfirmBlock()
		a.ChangeHeight()
		log.Info("[OnBlockReceived] received confirmed block")
		return
	}

	if blockchain.DefaultLedger.Blockchain.BlockHeight < b.Height { //new height block coming
		a.DposManager.ProcessHigherBlock(b)
	} else {
		log.Warn("a.Leger.LastBlock.Height", blockchain.DefaultLedger.Blockchain.BlockHeight, "b.Height", b.Height)
	}
}

func (a *Arbitrator) OnConfirmReceived(p *core.DPosProposalVoteSlot) {
	a.DposManager.Lock()
	defer a.DposManager.Unlock()

	log.Info("[OnConfirmReceived] started, hash:", p.Hash)
	defer log.Info("[OnConfirmReceived] end")

	a.DposManager.ConfirmBlock()
	a.ChangeHeight()
}

func (a *Arbitrator) ChangeHeight() { //called by leger later
	currentArbiter, err := blockchain.GetOnDutyArbiter()
	if err != nil {
		log.Error("Error occurred with change height: get current arbiter error.")
	}

	a.OnDutyArbitratorChanged(a.Name == common.BytesToHexString(currentArbiter))
}
