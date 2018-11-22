package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

const DefaultProposalMessageDataSize = 134 //33+32+4+65

type Proposal struct {
	Proposal core.DPosProposal
}

func (m *Proposal) CMD() string {
	return CmdReceivedProposal
}

func (m *Proposal) MaxLength() uint32 {
	return DefaultProposalMessageDataSize
}

func (m *Proposal) Serialize(w io.Writer) error {
	return m.Proposal.Serialize(w)
}

func (m *Proposal) Deserialize(r io.Reader) error {
	return m.Proposal.Deserialize(r)
}
