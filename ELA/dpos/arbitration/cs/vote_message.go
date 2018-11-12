package cs

import (
	"io"

	common2 "github.com/elastos/Elastos.ELA/dpos/arbitration/common"
)

type VoteMessage struct {
	Command string
	Vote    common2.DPosProposalVote
}

func (msg *VoteMessage) CMD() string {
	return msg.Command
}

func (msg *VoteMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *VoteMessage) Serialize(w io.Writer) error {
	return msg.Vote.Serialize(w)
}

func (msg *VoteMessage) Deserialize(r io.Reader) error {
	return msg.Vote.Deserialize(r)
}
