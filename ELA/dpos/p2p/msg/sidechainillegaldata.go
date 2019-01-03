package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types"
	msg2 "github.com/elastos/Elastos.ELA/p2p/msg"
)

type SidechainIllegalData struct {
	Data types.SidechainIllegalData
}

func (msg *SidechainIllegalData) CMD() string {
	return CmdSidechainIllegalData
}

func (msg *SidechainIllegalData) MaxLength() uint32 {
	return msg2.MaxBlockSize
}

func (msg *SidechainIllegalData) Serialize(w io.Writer) error {
	if err := msg.Data.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (msg *SidechainIllegalData) Deserialize(r io.Reader) error {
	if err := msg.Data.Deserialize(r); err != nil {
		return err
	}

	return nil
}
