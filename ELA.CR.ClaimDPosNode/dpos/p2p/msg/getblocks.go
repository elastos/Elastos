package msg

import (
	"github.com/elastos/Elastos.ELA/common"
	"io"
)

type GetBlocks struct {
	StartBlockHeight uint32
	EndBlockHeight   uint32
}

func (msg *GetBlocks) CMD() string {
	return CmdGetBlocks
}

func (msg *GetBlocks) MaxLength() uint32 {
	return 8
}

func (msg *GetBlocks) Serialize(w io.Writer) error {
	if err := common.WriteUint32(w, msg.StartBlockHeight); err != nil {
		return err
	}

	if err := common.WriteUint32(w, msg.EndBlockHeight); err != nil {
		return err
	}

	return nil
}

func (msg *GetBlocks) Deserialize(r io.Reader) error {
	var err error

	if msg.StartBlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	if msg.EndBlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	return nil
}
