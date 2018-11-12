package cs

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type PingMessage struct {
	Command string
	Height  uint32
}

func (msg *PingMessage) CMD() string {
	return msg.Command
}

func (msg *PingMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *PingMessage) Serialize(w io.Writer) error {
	if err := common.WriteUint32(w, msg.Height); err != nil {
		return err
	}

	return nil
}

func (msg *PingMessage) Deserialize(r io.Reader) error {
	var err error
	if msg.Height, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}
