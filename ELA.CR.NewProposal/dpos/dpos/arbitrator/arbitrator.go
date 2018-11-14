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

type Arbitrator struct {
	Name       string
	IsOnDuty   bool
	BlockCache ConsensusBlockCache

	DposManager IDposManager
}

func (a *Arbitrator) GetPublicKey() string {
	return a.Name
}

func (a *Arbitrator) OnBlockReceived(b *core.Block, confirmed bool) {
	a.DposManager.Lock()
	defer a.DposManager.Unlock()

	log.Info("[OnBlockReceived] start")
	defer log.Info("[OnBlockReceived] end")

	if confirmed {
		a.DposManager.ConfirmBlock()
		a.changeHeight()
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
	a.changeHeight()
}

func (a *Arbitrator) changeHeight() { //called by leger later
	currentArbiter, err := blockchain.GetOnDutyArbiter()
	if err != nil {
		log.Error("Error occurred with change height: get current arbiter error.")
	}

	a.onDutyArbitratorChanged(a.Name == common.BytesToHexString(currentArbiter))
}

func (a *Arbitrator) onDutyArbitratorChanged(onDuty bool) {
	if onDuty {
		log.Info("[onDutyArbitratorChanged] not onduty -> onduty")
	} else {
		log.Info("[onDutyArbitratorChanged] onduty -> not onduty")
	}
	a.IsOnDuty = onDuty
	a.DposManager.ChangeConsensus(onDuty)
}
