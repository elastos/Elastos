package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version   uint32
	Services  uint64
	PID       [33]byte
	Nonce     [32]byte
	Signature [64]byte
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 141 // 4+8+33+32+64
}

func (msg *Version) Serialize(w io.Writer) error {
	err := common.WriteElements(w, msg.Version, msg.Services)
	if err != nil {
		return err
	}

	if _, err = w.Write(msg.PID[:]); err != nil {
		return err
	}

	if _, err = w.Write(msg.Nonce[:]); err != nil {
		return err
	}

	_, err = w.Write(msg.Signature[:])
	return err
}

func (msg *Version) Deserialize(r io.Reader) error {
	err := common.ReadElements(r, &msg.Version, &msg.Services)
	if err != nil {
		return err
	}

	if _, err = io.ReadFull(r, msg.PID[:]); err != nil {
		return err
	}

	if _, err = io.ReadFull(r, msg.Nonce[:]); err != nil {
		return err
	}

	_, err = io.ReadFull(r, msg.Signature[:])
	return err
}

func NewVersion(pver uint32, services uint64, pid [33]byte,
	nonce [32]byte, signature [64]byte) *Version {

	return &Version{
		Version:   pver,
		Services:  services,
		PID:       pid,
		Nonce:     nonce,
		Signature: signature,
	}
}
