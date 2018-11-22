package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

type ResponseProposal struct {
	Proposal core.DPosProposal
}

func (msg *ResponseProposal) CMD() string {
	return CmdResponseProposal
}

func (msg *ResponseProposal) MaxLength() uint32 {
	return 33 + 32 + 4 + 65
}

func (msg *ResponseProposal) Serialize(w io.Writer) error {
	if err := msg.Proposal.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (msg *ResponseProposal) Deserialize(r io.Reader) error {
	if err := msg.Proposal.Deserialize(r); err != nil {
		return err
	}

	return nil
}
