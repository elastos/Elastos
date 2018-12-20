package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types"
)

const MaxIllegalVoteSize = 1000000

type IllegalVotes struct {
	Votes types.DposIllegalVotes
}

func (msg *IllegalVotes) CMD() string {
	return CmdIllegalVotes
}

func (msg *IllegalVotes) MaxLength() uint32 {
	return MaxIllegalVoteSize
}

func (msg *IllegalVotes) Serialize(w io.Writer) error {
	if err := msg.Votes.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (msg *IllegalVotes) Deserialize(r io.Reader) error {
	if err := msg.Votes.Deserialize(r); err != nil {
		return err
	}

	return nil
}
