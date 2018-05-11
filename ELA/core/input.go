package core

import (
	"bytes"
	"encoding/hex"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type Input struct {
	// Reference outpoint of this input
	Previous OutPoint

	// Sequence number
	Sequence uint32
}

func (i *Input) Serialize(w io.Writer) error {
	return WriteElements(w, i.Previous.TxID, i.Previous.Index, i.Sequence)
}

func (i *Input) Deserialize(r io.Reader) error {
	return ReadElements(r, &i.Previous.TxID, &i.Previous.Index, &i.Sequence)
}

func (i *Input) ReferKey() string {
	buf := new(bytes.Buffer)
	i.Previous.Serialize(buf)
	hash := Sha256D(buf.Bytes())
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
