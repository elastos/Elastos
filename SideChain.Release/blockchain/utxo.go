package blockchain

import (
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type UTXO struct {
	TxId  Uint256
	Index uint32
	Value Fixed64
}

func (uu *UTXO) Serialize(w io.Writer) {
	uu.TxId.Serialize(w)
	WriteUint32(w, uu.Index)
	uu.Value.Serialize(w)
}

func (uu *UTXO) Deserialize(r io.Reader) error {
	uu.TxId.Deserialize(r)

	index, err := ReadUint32(r)
	uu.Index = uint32(index)
	if err != nil {
		return err
	}

	uu.Value.Deserialize(r)

	return nil
}
