package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

const DefaultVoteMessageDataSize = 297 //164+67+1+65

type VoteMessage struct {
	Command string
	Vote    core.DPosProposalVote
}

func (msg *VoteMessage) CMD() string {
	return msg.Command
}

func (msg *VoteMessage) MaxLength() uint32 {
	return DefaultVoteMessageDataSize
}

func (msg *VoteMessage) Serialize(w io.Writer) error {
	return msg.Vote.Serialize(w)
}

func (msg *VoteMessage) Deserialize(r io.Reader) error {
	return msg.Vote.Deserialize(r)
}
