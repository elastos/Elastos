package manager

import (
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common/config"
)

const (
	// firstTimeoutFactor specified the factor first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 180 seconds)
	firstTimeoutFactor = uint32(1)

	// othersTimeoutFactor specified the factor after first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 12 hours)
	othersTimeoutFactor = uint32(240)
)

type ViewChangesCountDown struct {
	dispatcher  ProposalDispatcher
	consensus   *Consensus
	arbitrators interfaces.Arbitrators

	timeoutRefactor               uint32
	inactiveArbitratorsEliminated bool
}

func (c *ViewChangesCountDown) Reset() {
	c.inactiveArbitratorsEliminated = false
	c.timeoutRefactor = 0
}

func (c *ViewChangesCountDown) SetEliminated() {
	c.inactiveArbitratorsEliminated = true

	if c.timeoutRefactor == 0 {
		c.timeoutRefactor += firstTimeoutFactor
	} else {
		c.timeoutRefactor += othersTimeoutFactor
	}
}

func (c *ViewChangesCountDown) IsTimeOut() bool {
	//todo improve me when height versions refactor is done
	if c.dispatcher.CurrentHeight() <= config.Parameters.HeightVersions[3] {
		return false
	}

	return c.consensus.GetViewOffset() >= c.arbitrators.GetArbitersCount()*c.
		timeoutRefactor
}
