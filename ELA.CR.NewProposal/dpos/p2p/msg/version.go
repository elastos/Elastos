package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version  uint32
	Services uint64
	Port     uint16
	PID      [33]byte
	Nonce    [32]byte
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 79 // 4+8+2+33+32
}

func (msg *Version) Serialize(w io.Writer) error {
	return common.WriteElements(w,
		msg.Version,
		msg.Services,
		msg.Port,
		msg.PID,
		msg.Nonce)
}

func (msg *Version) Deserialize(r io.Reader) error {
	return common.ReadElements(r,
		&msg.Version,
		&msg.Services,
		&msg.Port,
		&msg.PID,
		&msg.Nonce)
}

func NewVersion(pver uint32, services uint64, port uint16, pid [33]byte,
	nonce [32]byte) *Version {
	return &Version{
		Version:  pver,
		Services: services,
		Port:     port,
		PID:      pid,
		Nonce:    nonce,
	}
}
