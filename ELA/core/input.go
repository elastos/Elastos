package core

import (
	"encoding/hex"
	"fmt"
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
	return WriteElements(w, &i.Previous.TxID, i.Previous.Index, i.Sequence)
}

func (i *Input) Deserialize(r io.Reader) error {
	return ReadElements(r, &i.Previous.TxID, &i.Previous.Index, &i.Sequence)
}

func (i *Input) ReferKey() string {
	return hex.EncodeToString(i.Previous.Bytes())
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

func (i Input) String() string {
	return fmt.Sprint("{",
		"TxId: ", i.Previous.TxID.String(),
		" Index: ", fmt.Sprint(i.Previous.Index),
		" Sequence: ", fmt.Sprint(i.Sequence),
		"}")
}
