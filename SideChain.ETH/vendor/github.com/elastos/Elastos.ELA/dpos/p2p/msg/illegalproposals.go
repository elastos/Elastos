package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const MaxIllegalProposalSize = 1000000

type IllegalProposals struct {
	Proposals payload.DPOSIllegalProposals
}

func (msg *IllegalProposals) CMD() string {
	return CmdIllegalProposals
}

func (msg *IllegalProposals) MaxLength() uint32 {
	return MaxIllegalProposalSize
}

func (msg *IllegalProposals) Serialize(w io.Writer) error {
	if err := msg.Proposals.Serialize(w,
		payload.IllegalProposalVersion); err != nil {
		return err
	}

	return nil
}

func (msg *IllegalProposals) Deserialize(r io.Reader) error {
	if err := msg.Proposals.Deserialize(r,
		payload.IllegalProposalVersion); err != nil {
		return err
	}

	return nil
}
