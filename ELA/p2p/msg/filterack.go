package msg

import (
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure FilterAck implement p2p.Message interface.
var _ p2p.Message = (*FilterAck)(nil)

type FilterAck struct{ empty }

func (msg *FilterAck) CMD() string {
	return p2p.CmdFilterAck
}

func NewFilterAck() *FilterAck { return &FilterAck{} }
