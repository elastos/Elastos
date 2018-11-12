package cs

import (
	"io"

	"github.com/elastos/Elastos.ELA/dpos/chain"
)

type ConfirmMessage struct {
	Command  string
	Proposal chain.ProposalVoteSlot
}

func (msg *ConfirmMessage) CMD() string {
	return msg.Command
}

func (msg *ConfirmMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *ConfirmMessage) Serialize(w io.Writer) error {
	return msg.Proposal.Serialize(w)
}

func (msg *ConfirmMessage) Deserialize(r io.Reader) error {
	return msg.Proposal.Deserialize(r)
}
