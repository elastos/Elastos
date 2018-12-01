package manager

import (
	"time"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	consensusReady = iota
	consensusRunning
)

type Consensus interface {
	AbnormalRecovering

	IsRunning() bool
	SetRunning()
	IsReady() bool
	SetReady()

	IsOnDuty() bool
	SetOnDuty(onDuty bool)

	IsArbitratorOnDuty(arbitrator string) bool
	GetOnDutyArbitrator() string

	StartConsensus(b *types.Block)
	ProcessBlock(b *types.Block)

	ChangeView()
	TryChangeView() bool
	GetViewOffset() uint32
}

type consensus struct {
	consensusStatus uint32
	viewOffset      uint32

	manager     DposManager
	currentView view
}

func NewConsensus(manager DposManager, tolerance time.Duration, viewListener ViewListener) Consensus {
	c := &consensus{
		consensusStatus: consensusReady,
		viewOffset:      0,
		manager:         manager,
		currentView:     view{signTolerance: tolerance, listener: viewListener},
	}

	return c
}

func (c *consensus) IsOnDuty() bool {
	return c.currentView.IsOnDuty()
}

func (c *consensus) SetOnDuty(onDuty bool) {
	c.currentView.SetOnDuty(onDuty)
}

func (c *consensus) SetRunning() {
	c.consensusStatus = consensusRunning
	c.resetViewOffset()
}

func (c *consensus) SetReady() {
	c.consensusStatus = consensusReady
	c.resetViewOffset()
}

func (c *consensus) IsRunning() bool {
	return c.consensusStatus == consensusRunning
}

func (c *consensus) IsReady() bool {
	return c.consensusStatus == consensusReady
}

func (c *consensus) IsArbitratorOnDuty(arbitrator string) bool {
	return c.GetOnDutyArbitrator() == arbitrator
}

func (c *consensus) GetOnDutyArbitrator() string {
	a := blockchain.DefaultLedger.Arbitrators.GetNextOnDutyArbitrator(c.viewOffset)
	return common.BytesToHexString(a)
}

func (c *consensus) StartConsensus(b *types.Block) {
	log.Info("[StartConsensus] consensus start")
	defer log.Info("[StartConsensus] consensus end")

	now := time.Now()
	c.manager.GetBlockCache().Reset()
	c.SetRunning()

	c.manager.GetBlockCache().AddValue(b.Hash(), b)
	c.currentView.ResetView(now)

}

func (c *consensus) GetViewOffset() uint32 {
	return c.viewOffset
}

func (c *consensus) ProcessBlock(b *types.Block) {
	c.manager.GetBlockCache().AddValue(b.Hash(), b)
}

func (c *consensus) ChangeView() {
	c.currentView.ChangeView(&c.viewOffset)
}

func (c *consensus) TryChangeView() bool {
	if c.IsRunning() {
		return c.currentView.TryChangeView(&c.viewOffset)
	}
	return false
}

func (c *consensus) CollectConsensusStatus(height uint32, status *msg.ConsensusStatus) error {
	status.ConsensusStatus = c.consensusStatus
	status.ViewOffset = c.viewOffset
	status.ViewStartTime = c.currentView.GetViewStartTime()
	log.Info("[CollectConsensusStatus] status.ConsensusStatus:", status.ConsensusStatus)
	return nil
}

func (c *consensus) RecoverFromConsensusStatus(status *msg.ConsensusStatus) error {
	log.Info("[RecoverFromConsensusStatus] status.ConsensusStatus:", status.ConsensusStatus)
	c.consensusStatus = status.ConsensusStatus
	c.viewOffset = status.ViewOffset
	c.currentView.ResetView(status.ViewStartTime)
	return nil
}

func (c *consensus) resetViewOffset() {
	c.viewOffset = 0
}
