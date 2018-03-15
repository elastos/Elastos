package transaction

import (
	"io"
	"bytes"

	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialization"
)

type OutPoint struct {
	TxID  Uint256
	Index uint16
}

func (op *OutPoint) Serialize(w io.Writer) error {
	_, err := op.TxID.Serialize(w)
	if err != nil {
		return err
	}

	err = serialization.WriteUint16(w, op.Index)
	if err != nil {
		return err
	}

	return nil
}

func (op *OutPoint) Deserialize(r io.Reader) error {
	err := op.TxID.Deserialize(r)
	if err != nil {
		return err
	}

	op.Index, err = serialization.ReadUint16(r)
	if err != nil {
		return err
	}

	return nil
}

func (op *OutPoint) Bytes() []byte {
	buf := new(bytes.Buffer)
	op.Serialize(buf)
	return buf.Bytes()
}

func NewOutPoint(txId Uint256, index uint16) *OutPoint {
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
