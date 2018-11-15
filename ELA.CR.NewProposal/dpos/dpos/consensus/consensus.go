package consensus

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/dpos/dpos/arbitrator"
	"github.com/elastos/Elastos.ELA/dpos/dpos/view"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	consensusReady = iota
	consensusRunning
)

type Consensus struct {
	consensusStatus uint32
	viewOffset      uint32

	currentView view.View
	//todo uncomment me later
	//statusLock  sync.Mutex
}

func (c *Consensus) Initialize(tolerance time.Duration, viewListener view.ViewListener) {
	c.currentView = view.View{}
	c.currentView.Initialize(tolerance, viewListener)
}

func (c *Consensus) IsOnDuty() bool {
	return c.currentView.IsOnDuty()
}

func (c *Consensus) SetOnDuty(onDuty bool) {
	c.currentView.SetOnDuty(onDuty)
}

func (c *Consensus) RunWithStatusCondition(condition bool, closure func()) {
	//todo uncomment me later
	//c.statusLock.Lock()
	//defer c.statusLock.Unlock()

	if condition {
		closure()
	}
}

func (c *Consensus) RunWithAllStatusConditions(ready func(), running func()) {
	//todo uncomment me later
	//c.statusLock.Lock()
	//defer c.statusLock.Unlock()

	switch c.consensusStatus {
	case consensusReady:
		ready()
	case consensusRunning:
		running()
	}
}

func (c *Consensus) SetRunning() {
	c.consensusStatus = consensusRunning
	c.resetViewOffset()
}

func (c *Consensus) SetReady() {
	c.consensusStatus = consensusReady
	c.resetViewOffset()
}

func (c *Consensus) IsRunning() bool {
	return c.consensusStatus == consensusRunning
}

func (c *Consensus) IsReady() bool {
	return c.consensusStatus == consensusReady
}

func (c *Consensus) IsArbitratorOnDuty(arbitrator string) bool {
	return c.GetOnDutyArbitrator() == arbitrator
}

func (c *Consensus) GetOnDutyArbitrator() string {
	a, _ := blockchain.GetNextOnDutyArbiter(c.viewOffset)
	return common.BytesToHexString(a)
}

func (c *Consensus) StartConsensus(b *core.Block) {
	now := time.Now()
	ArbitratorSingleton.BlockCache.Reset()
	c.SetRunning()

	ArbitratorSingleton.BlockCache.AddValue(b.Hash(), b)
	c.currentView.ResetView(now)
	log.Info("[StartConsensus] consensus started")
}

func (c *Consensus) GetViewOffset() uint32 {
	return c.viewOffset
}

func (c *Consensus) ProcessBlock(b *core.Block) {
	ArbitratorSingleton.BlockCache.AddValue(b.Hash(), b)
}

func (c *Consensus) ChangeView() {
	c.currentView.ChangeView(&c.viewOffset)
}

func (c *Consensus) TryChangeView() bool {
	return c.currentView.TryChangeView(&c.viewOffset)
}

func (c *Consensus) CollectConsensusStatus(height uint32, status *msg.ConsensusStatus) error {
	status.ConsensusStatus = c.consensusStatus
	status.ViewOffset = c.viewOffset
	status.ViewStartTime = c.currentView.GetViewStartTime()
	return nil
}

func (c *Consensus) RecoverFromConsensusStatus(status *msg.ConsensusStatus) error {
	c.consensusStatus = status.ConsensusStatus
	c.viewOffset = status.ViewOffset
	c.currentView.ResetView(status.ViewStartTime)
	return nil
}

func (c *Consensus) resetViewOffset() {
	c.viewOffset = 0
}
