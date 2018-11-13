package msg

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure VerAck implement p2p.Message interface.
var _ p2p.Message = (*VerAck)(nil)

type VerAck struct{ empty }

func (msg *VerAck) CMD() string {
	return p2p.CmdVerAck
}

func NewVerAck() *VerAck { return &VerAck{} }
