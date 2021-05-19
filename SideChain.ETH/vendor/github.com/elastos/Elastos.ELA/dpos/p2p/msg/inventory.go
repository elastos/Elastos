package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Inventory implement p2p.Message interface.
var _ p2p.Message = (*Inventory)(nil)

type Inventory struct {
	BlockHash common.Uint256
}

func NewInventory(blockHash common.Uint256) *Inventory {
	inv := new(Inventory)
	inv.BlockHash = blockHash
	return inv
}

func (msg *Inventory) CMD() string {
	return CmdInv
}

func (msg *Inventory) MaxLength() uint32 {
	return 32
}

func (msg *Inventory) Serialize(writer io.Writer) error {
	return msg.BlockHash.Serialize(writer)
}

func (msg *Inventory) Deserialize(reader io.Reader) error {
	return msg.BlockHash.Deserialize(reader)
}
