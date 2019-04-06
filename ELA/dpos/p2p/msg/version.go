package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	PID     [33]byte
	Target  [16]byte
	Nonce   [16]byte
	Port    uint16
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 67 // 33+16+16+2
}

func (msg *Version) Serialize(w io.Writer) error {
	return common.WriteElements(w,
		msg.PID,
		msg.Target,
		msg.Nonce,
		msg.Port)
}

func (msg *Version) Deserialize(r io.Reader) error {
	return common.ReadElements(r,
		&msg.PID,
		&msg.Target,
		&msg.Nonce,
		&msg.Port)
}

func NewVersion(pid [33]byte, target, nonce [16]byte, port uint16) *Version {
	return &Version{
		PID:    pid,
		Target: target,
		Nonce:  nonce,
		Port:   port,
	}
}
