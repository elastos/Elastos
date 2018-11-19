package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

const DefaultProposalMessageDataSize = 164 //67+32+65

type ProposalMessage struct {
	Proposal core.DPosProposal
}

func (m *ProposalMessage) CMD() string {
	return ReceivedProposal
}

func (m *ProposalMessage) MaxLength() uint32 {
	return DefaultProposalMessageDataSize
}

func (m *ProposalMessage) Serialize(w io.Writer) error {
	return m.Proposal.Serialize(w)
}

func (m *ProposalMessage) Deserialize(r io.Reader) error {
	return m.Proposal.Deserialize(r)
}
