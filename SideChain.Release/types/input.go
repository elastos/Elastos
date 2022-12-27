package types

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type Input struct {
	// Reference outpoint of this input
	Previous OutPoint

	// Sequence number
	Sequence uint32
}

func (i *Input) Serialize(w io.Writer) error {
	return common.WriteElements(w, &i.Previous.TxID, i.Previous.Index, i.Sequence)
}

func (i *Input) Deserialize(r io.Reader) error {
	return common.ReadElements(r, &i.Previous.TxID, &i.Previous.Index, &i.Sequence)
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

func (i Input) String() string {
	return "{\n\t\t" +
		"tx id: " + i.Previous.TxID.String() + "\n\t\t" +
		"index: " + fmt.Sprint(i.Previous.Index) + "\n\t\t" +
		"sequence: " + fmt.Sprint(i.Sequence) + "\n\t\t" +
		"}"
}
