package manager

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

const (
	// firstTimeoutFactor specified the factor first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 3 minutes)
	firstTimeoutFactor = uint32(1)

	// othersTimeoutFactor specified the factor after first dynamic change
	// arbitrators in one consensus
	// (timeout will occurred in about 6 minutes)
	othersTimeoutFactor = uint32(2)
)

type ViewChangesCountDown struct {
	dispatcher  *ProposalDispatcher
	consensus   *Consensus
	arbitrators state.Arbitrators

	handledPayloadHashes map[common.Uint256]struct{}
	timeoutRefactor      uint32
	startViewOffset      uint32
}

func (c *ViewChangesCountDown) Reset(start uint32) {
	c.handledPayloadHashes = make(map[common.Uint256]struct{})
	c.timeoutRefactor = firstTimeoutFactor
	c.startViewOffset = start
}

func (c *ViewChangesCountDown) SetEliminated(hash common.Uint256) bool {
	if _, ok := c.handledPayloadHashes[hash]; !ok {
		c.handledPayloadHashes[hash] = struct{}{}
		c.timeoutRefactor += othersTimeoutFactor
		return true
	}
	return false
}

func (c *ViewChangesCountDown) IsTimeOut() bool {
	if blockchain.DefaultLedger.Blockchain.GetHeight()+1 <
		c.dispatcher.cfg.ChainParams.PublicDPOSHeight ||
		c.arbitrators.IsInactiveMode() {
		return false
	}

	if c.arbitrators.GetCRCArbitersCount() == c.arbitrators.GetArbitersCount() {
		return false
	}

	return c.consensus.GetViewOffset()-c.startViewOffset >=
		uint32(c.arbitrators.GetArbitersCount())*c.timeoutRefactor
}
