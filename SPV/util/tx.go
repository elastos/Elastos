package util

import (
	"encoding/binary"
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

// Tx is a data structure used in database.
type Tx struct {
	// The origin transaction data.
	core.Transaction

	// The block height that this transaction
	// belongs to.
	Height uint32
}

func NewTx(tx core.Transaction, height uint32) *Tx {
	return &Tx{
		Transaction: tx,
		Height:      height,
	}
}

func (t *Tx) Serialize(buf io.Writer) error {
	if err := t.Transaction.Serialize(buf); err != nil {
		return err
	}
	return binary.Write(buf, binary.LittleEndian, t.Height)
}

func (t *Tx) Deserialize(reader io.Reader) error {
	if err := t.Transaction.Deserialize(reader); err != nil {
		return err
	}
	return binary.Read(reader, binary.LittleEndian, &t.Height)
}
