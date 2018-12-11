package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure GetAddr implement p2p.Message interface.
var _ p2p.Message = (*GetAddr)(nil)

type GetAddr struct{ empty }

func (msg *GetAddr) CMD() string {
	return p2p.CmdGetAddr
}

func NewGetAddr() *GetAddr { return &GetAddr{} }
