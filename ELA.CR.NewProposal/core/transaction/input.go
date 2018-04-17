package transaction

import (
	"Elastos.ELA/common/serialize"
	"io"
	"bytes"
	"Elastos.ELA/common"
	"encoding/hex"
)

type Input struct {
	// Reference outpoint of this input
	Previous OutPoint

	// Sequence number
	Sequence uint32
}

func (i *Input) Serialize(w io.Writer) error {
	return serialize.WriteElements(w, i.Previous.TxID, i.Previous.Index, i.Sequence)
}

func (i *Input) Deserialize(r io.Reader) error {
	return serialize.ReadElements(r, &i.Previous.TxID, &i.Previous.Index, &i.Sequence)
}

func (i *Input) ReferKey() string {
	buf := new(bytes.Buffer)
	i.Previous.Serialize(buf)
	hash := common.Sha256D(buf.Bytes())
	return hex.EncodeToString(hash[:])
}

func (i *Input) IsEqual(o Input) bool {
	if !i.Previous.IsEqual(o.Previous) {
		return false
	}
	if i.Sequence != o.Sequence {
		return false
	}
	return true
}
