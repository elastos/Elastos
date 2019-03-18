package msg

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
)

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
	return pact.MaxBlockSize
}
