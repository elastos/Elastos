package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

type ProposalMessage struct {
	Proposal core.DPosProposal
}

func (m *ProposalMessage) CMD() string {
	return ReceivedProposal
}

func (m *ProposalMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (m *ProposalMessage) Serialize(w io.Writer) error {
	return m.Proposal.Serialize(w)
}

func (m *ProposalMessage) Deserialize(r io.Reader) error {
	return m.Proposal.Deserialize(r)
}
