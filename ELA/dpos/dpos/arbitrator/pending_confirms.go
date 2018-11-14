package arbitrator

import (
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type PendingConfirms struct {
	Confirms map[common.Uint256]*core.DPosProposalVoteSlot
}

func (c *PendingConfirms) AppendPendingConfirms(b *core.DPosProposalVoteSlot) {
	c.Confirms[b.Hash] = b
}

func (c *PendingConfirms) GetPendingConfirms(blockHash common.Uint256) (*core.DPosProposalVoteSlot, bool) {
	confirm, ok := c.Confirms[blockHash]
	return confirm, ok
}
