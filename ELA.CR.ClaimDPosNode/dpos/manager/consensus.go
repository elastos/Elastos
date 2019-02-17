package manager

import (
	"bytes"
	"time"

	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
)

const (
	consensusReady = iota
	consensusRunning
)

type Consensus struct {
	consensusStatus uint32
	viewOffset      uint32

	manager     DposManager
	currentView view
}

func NewConsensus(manager DposManager, tolerance time.Duration,
	viewListener ViewListener) *Consensus {
	c := &Consensus{
		consensusStatus: consensusReady,
		viewOffset:      0,
		manager:         manager,
		currentView:     view{signTolerance: tolerance, listener: viewListener, arbitrators: manager.GetArbitrators()},
	}

	return c
}

func (c *Consensus) IsOnDuty() bool {
	return c.currentView.IsOnDuty()
}

func (c *Consensus) SetOnDuty(onDuty bool) {
	c.currentView.SetOnDuty(onDuty)
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

func (c *Consensus) IsArbitratorOnDuty(arbitrator []byte) bool {
	return bytes.Equal(c.GetOnDutyArbitrator(), arbitrator)
}

func (c *Consensus) GetOnDutyArbitrator() []byte {
	return c.manager.GetArbitrators().GetNextOnDutyArbitrator(c.viewOffset)
}

func (c *Consensus) StartConsensus(b *types.Block) {
	log.Info("[StartConsensus] consensus start")
	defer log.Info("[StartConsensus] consensus end")

	now := time.Now()
	c.manager.GetBlockCache().Reset()
	c.SetRunning()

	c.manager.GetBlockCache().AddValue(b.Hash(), b)
	c.currentView.ResetView(now)

}

func (c *Consensus) GetViewOffset() uint32 {
	return c.viewOffset
}

func (c *Consensus) ProcessBlock(b *types.Block) {
	c.manager.GetBlockCache().AddValue(b.Hash(), b)
}

func (c *Consensus) ChangeView() {
	c.currentView.ChangeView(&c.viewOffset)
}

func (c *Consensus) TryChangeView() bool {
	if c.IsRunning() {
		return c.currentView.TryChangeView(&c.viewOffset)
	}
	return false
}

func (c *Consensus) CollectConsensusStatus(height uint32, status *msg.ConsensusStatus) error {
	status.ConsensusStatus = c.consensusStatus
	status.ViewOffset = c.viewOffset
	status.ViewStartTime = c.currentView.GetViewStartTime()
	log.Info("[CollectConsensusStatus] status.ConsensusStatus:", status.ConsensusStatus)
	return nil
}

func (c *Consensus) RecoverFromConsensusStatus(status *msg.ConsensusStatus) error {
	log.Info("[RecoverFromConsensusStatus] status.ConsensusStatus:", status.ConsensusStatus)
	c.consensusStatus = status.ConsensusStatus
	c.viewOffset = status.ViewOffset
	c.currentView.ResetView(status.ViewStartTime)
	return nil
}

func (c *Consensus) resetViewOffset() {
	c.viewOffset = 0
}
