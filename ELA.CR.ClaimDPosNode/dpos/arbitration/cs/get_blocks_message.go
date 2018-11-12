package cs

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"io"
)

type GetBlocksMessage struct {
	Command          string
	StartBlockHeight uint32
	EndBlockHeight   uint32
}

func (msg *GetBlocksMessage) CMD() string {
	return msg.Command
}

func (msg *GetBlocksMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *GetBlocksMessage) Serialize(w io.Writer) error {
	if err := common.WriteUint32(w, msg.StartBlockHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, msg.EndBlockHeight); err != nil {
		return err
	}

	return nil
}

func (msg *GetBlocksMessage) Deserialize(r io.Reader) error {
	var err error

	if msg.StartBlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	if msg.EndBlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}
