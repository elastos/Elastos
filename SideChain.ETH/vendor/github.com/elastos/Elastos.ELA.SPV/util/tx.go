package util

import (
	"bytes"
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
)

// Tx is a data structure used in database.
type Tx struct {
	// The transaction hash.
	Hash common.Uint256

	// The block height that this transaction
	// belongs to.
	Height uint32

	// The time the transaction was first seen
	Timestamp time.Time

	// Transaction
	RawData []byte
}

func NewTx(tx Transaction, height uint32) *Tx {
	buf := new(bytes.Buffer)
	tx.Serialize(buf)
	return &Tx{
		Hash:      tx.Hash(),
		Height:    height,
		Timestamp: time.Now(),
		RawData:   buf.Bytes(),
	}
}

func (t *Tx) Serialize(w io.Writer) error {
	if err := t.Hash.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint32(w, t.Height); err != nil {
		return err
	}

	err := common.WriteUint64(w, uint64(t.Timestamp.Unix()))
	if err != nil {
		return err
	}

	return common.WriteVarBytes(w, t.RawData)
}

func (t *Tx) Deserialize(r io.Reader) error {
	if err := t.Hash.Deserialize(r); err != nil {
		return err
	}

	var err error
	t.Height, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	timestamp, err := common.ReadUint64(r)
	if err != nil {
		return err
	}
	t.Timestamp = time.Unix(int64(timestamp), 0)

	t.RawData, err = common.ReadVarBytes(r, pact.MaxBlockSize,
		"Tx RawData")
	return err
}
