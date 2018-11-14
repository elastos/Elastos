package cs

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

type ProposalMessage struct {
	Command  string
	Proposal core.DPosProposal
}

func (msg *ProposalMessage) CMD() string {
	return msg.Command
}

func (msg *ProposalMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *ProposalMessage) Serialize(w io.Writer) error {
	return msg.Proposal.Serialize(w)
}

func (msg *ProposalMessage) Deserialize(r io.Reader) error {
	return msg.Proposal.Deserialize(r)
}
