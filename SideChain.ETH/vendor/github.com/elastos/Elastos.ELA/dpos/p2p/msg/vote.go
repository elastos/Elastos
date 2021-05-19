package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const DefaultVoteMessageDataSize = 297 //164+67+1+65

type Vote struct {
	Command string
	Vote    payload.DPOSProposalVote
}

func (msg *Vote) CMD() string {
	return msg.Command
}

func (msg *Vote) MaxLength() uint32 {
	return DefaultVoteMessageDataSize
}

func (msg *Vote) Serialize(w io.Writer) error {
	return msg.Vote.Serialize(w)
}

func (msg *Vote) Deserialize(r io.Reader) error {
	return msg.Vote.Deserialize(r)
}
