package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const MaxBlockSize = 8000000

const MaxTxPerBlock = 100000

// Ensure Block implement p2p.Message interface.
var _ p2p.Message = (*Block)(nil)

type Block struct {
	common.Serializable
}

func NewBlock(block common.Serializable) *Block {
	return &Block{Serializable: block}
}

func (msg *Block) CMD() string {
	return p2p.CmdBlock
}

func (msg *Block) MaxLength() uint32 {
	return MaxBlockSize
}
