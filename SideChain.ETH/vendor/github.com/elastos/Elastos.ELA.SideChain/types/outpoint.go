package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type OutPoint struct {
	TxID  common.Uint256
	Index uint16
}

func (o *OutPoint) IsEqual(t OutPoint) bool {
	if !o.TxID.IsEqual(t.TxID) {
		return false
	}
	if o.Index != t.Index {
		return false
	}
	return true
}

func (o *OutPoint) Serialize(w io.Writer) error {
	return common.WriteElements(w, &o.TxID, o.Index)
}

func (o *OutPoint) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &o.TxID, &o.Index)
}

func (o *OutPoint) Bytes() []byte {
	buf := new(bytes.Buffer)
	o.Serialize(buf)
	return buf.Bytes()
}

func NewOutPoint(txId common.Uint256, index uint16) *OutPoint {
	return &OutPoint{
		TxID:  txId,
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
