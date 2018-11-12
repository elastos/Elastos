package arbitrator

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type PendingConfirms struct {
	Confirms map[common.Uint256]*msg.DPosProposalVoteSlot
}

func (c *PendingConfirms) AppendPendingConfirms(b *msg.DPosProposalVoteSlot) {
	c.Confirms[b.Hash] = b
}

func (c *PendingConfirms) GetPendingConfirms(blockHash common.Uint256) (*msg.DPosProposalVoteSlot, bool) {
	confirm, ok := c.Confirms[blockHash]
	return confirm, ok
}
