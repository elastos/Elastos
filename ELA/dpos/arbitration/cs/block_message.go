package cs

import (
	"io"

	"github.com/elastos/Elastos.ELA/dpos/chain"
)

type BlockMessage struct {
	Command string
	Block   chain.Block
}

func (msg *BlockMessage) CMD() string {
	return msg.Command
}

func (msg *BlockMessage) MaxLength() uint32 {
	return 0
}

func (msg *BlockMessage) Serialize(w io.Writer) error {
	return msg.Block.Serialize(w)
}

func (msg *BlockMessage) Deserialize(r io.Reader) error {
	return msg.Block.Deserialize(r)
}
