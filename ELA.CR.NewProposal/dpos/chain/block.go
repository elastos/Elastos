package chain

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Block struct {
	Hash      common.Uint256
	PrevHash  common.Uint256
	Height    uint32
	TimeStamp uint64
}

func (b *Block) Serialize(w io.Writer) error {
	if err := b.Hash.Serialize(w); err != nil {
		return err
	}

	if err := b.PrevHash.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint32(w, b.Height); err != nil {
		return err
	}

	if err := common.WriteUint64(w, b.TimeStamp); err != nil {
		return err
	}

	return nil
}

func (b *Block) Deserialize(r io.Reader) error {
	if err := b.Hash.Deserialize(r); err != nil {
		return err
	}

	if err := b.PrevHash.Deserialize(r); err != nil {
		return err
	}

	height, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	b.Height = height

	time, err := common.ReadUint64(r)
	if err != nil {
		return err
	}
	b.TimeStamp = time

	return nil
}
