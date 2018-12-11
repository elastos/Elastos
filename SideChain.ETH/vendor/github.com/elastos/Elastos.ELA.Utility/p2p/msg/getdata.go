package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure GetData implement p2p.Message interface.
var _ p2p.Message = (*GetData)(nil)

type GetData struct {
	Inv
}

func NewGetData() *GetData {
	return &GetData{}
}

func (msg *GetData) CMD() string {
	return p2p.CmdGetData
}
