package db

import (
	"encoding/binary"
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

type StoreTx struct {
	Height uint32
	core.Transaction
}

func NewStoreTx(tx *core.Transaction, height uint32) *StoreTx {
	return &StoreTx{
		Height:      height,
		Transaction: *tx,
	}
}

func (t *StoreTx) Serialize(buf io.Writer) error {
	if err := binary.Write(buf, binary.LittleEndian, t.Height); err != nil {
		return err
	}
	return t.Transaction.Serialize(buf)
}

func (t *StoreTx) Deserialize(reader io.Reader) error {
	if err := binary.Read(reader, binary.LittleEndian, &t.Height); err != nil {
		return err
	}
	return t.Transaction.Deserialize(reader)
}
