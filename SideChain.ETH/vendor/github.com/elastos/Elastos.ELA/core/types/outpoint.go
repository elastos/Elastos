package types

import (
	"bytes"
	"encoding/hex"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type OutPoint struct {
	TxID  common.Uint256
	Index uint16
}

func (op *OutPoint) IsEqual(o OutPoint) bool {
	if !op.TxID.IsEqual(o.TxID) {
		return false
	}
	if op.Index != o.Index {
		return false
	}
	return true
}

func (op *OutPoint) Serialize(w io.Writer) error {
	return common.WriteElements(w, &op.TxID, op.Index)
}

func (op *OutPoint) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &op.TxID, &op.Index)
}

func (op *OutPoint) Bytes() []byte {
	buf := new(bytes.Buffer)
	op.Serialize(buf)
	return buf.Bytes()
}

func (op *OutPoint) ReferKey() string {
	return hex.EncodeToString(op.Bytes())
}

func NewOutPoint(txID common.Uint256, index uint16) *OutPoint {
	return &OutPoint{
		TxID:  txID,
		Index: index,
	}
}

func OutPointFromBytes(value []byte) (*OutPoint, error) {
	outPoint := new(OutPoint)
	err := outPoint.Deserialize(bytes.NewReader(value))
	if err != nil {
		return nil, err
	}

	return outPoint, nil
}
