package manager

import (
	"github.com/elastos/Elastos.ELA/dpos/state"
)

const (
	// firstTimeoutFactor specified the factor first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 9 minutes)
	firstTimeoutFactor = uint32(3)

	// othersTimeoutFactor specified the factor after first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 1 hours)
	othersTimeoutFactor = uint32(20)
)

type ViewChangesCountDown struct {
	dispatcher  *ProposalDispatcher
	consensus   *Consensus
	arbitrators state.Arbitrators

	timeoutRefactor uint32
}

func (c *ViewChangesCountDown) Reset() {
	c.timeoutRefactor = firstTimeoutFactor
}

func (c *ViewChangesCountDown) SetEliminated() {
	c.timeoutRefactor += othersTimeoutFactor
}

func (c *ViewChangesCountDown) IsTimeOut() bool {
	if c.dispatcher.CurrentHeight() <=
		c.dispatcher.cfg.ChainParams.PublicDPOSHeight {
		return false
	}

	return c.consensus.GetViewOffset() >=
		uint32(c.arbitrators.GetArbitersCount())*c.timeoutRefactor
}
