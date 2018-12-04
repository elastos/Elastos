package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type GetAddr struct{}

func (msg *GetAddr) CMD() string {
	return p2p.CmdGetAddr
}

func (msg *GetAddr) MaxLength() uint32 {
	return 0
}

func (msg *GetAddr) Serialize(io.Writer) error {
	return nil
}

func (msg *GetAddr) Deserialize(io.Reader) error {
	return nil
}
