package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const MaxBlockSize = 8000000

const MaxTxPerBlock = 100000

type Block struct {
	Block common.Serializable
}

func NewBlock(block common.Serializable) *Block {
	return &Block{Block: block}
}

func (msg *Block) CMD() string {
	return p2p.CmdBlock
}

func (msg *Block) MaxLength() uint32 {
	return MaxBlockSize
}

func (msg *Block) Serialize(writer io.Writer) error {
	return msg.Block.Serialize(writer)
}

func (msg *Block) Deserialize(reader io.Reader) error {
	return msg.Block.Deserialize(reader)
}
