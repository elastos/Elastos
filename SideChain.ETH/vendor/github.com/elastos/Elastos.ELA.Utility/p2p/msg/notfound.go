package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure NotFound implement p2p.Message interface.
var _ p2p.Message = (*NotFound)(nil)

type NotFound struct {
	Inv
}

func NewNotFound() *NotFound {
	return &NotFound{}
}

func (msg *NotFound) CMD() string {
	return p2p.CmdNotFound
}
