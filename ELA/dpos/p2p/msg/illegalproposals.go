package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

const MaxIllegalProposalSize = 1000000

type IllegalProposals struct {
	Proposals core.DposIllegalProposals
}

func (msg *IllegalProposals) CMD() string {
	return CmdIllegalProposals
}

func (msg *IllegalProposals) MaxLength() uint32 {
	return MaxIllegalProposalSize
}

func (msg *IllegalProposals) Serialize(w io.Writer) error {
	if err := msg.Proposals.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (msg *IllegalProposals) Deserialize(r io.Reader) error {
	if err := msg.Proposals.Deserialize(r); err != nil {
		return err
	}

	return nil
}
