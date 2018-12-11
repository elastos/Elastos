package msg

import "github.com/elastos/Elastos.ELA.Utility/p2p"

// Ensure Inventory implement p2p.Message interface.
var _ p2p.Message = (*Inventory)(nil)

// Inventory is the same to Inv message.
type Inventory struct {
	Inv
}

func NewInventory() *Inventory {
	return &Inventory{}
}
