package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type GetData struct {
	Inventory
}

func NewGetData() *GetData {
	msg := &GetData{
		Inventory: *NewInventory(),
	}
	return msg
}

func (msg *GetData) CMD() string {
	return p2p.CmdGetData
}
