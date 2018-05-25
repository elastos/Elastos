package core

import (
	"bytes"
	"encoding/hex"
	"io"
	"fmt"

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

func (o Input) String() string {
	return "{\n\t\t" +
		"tx id: " + o.Previous.TxID.String() + "\n\t\t" +
		"index: " + fmt.Sprint(o.Previous.Index) + "\n\t\t" +
		"sequence: " + fmt.Sprint(o.Sequence) + "\n\t\t" +
		"}"
}