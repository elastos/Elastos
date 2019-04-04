package types

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type UTXO struct {
	TxId  common.Uint256
	Index uint32
	Value common.Fixed64
}

func (u *UTXO) Serialize(w io.Writer) error {
	if err := u.TxId.Serialize(w); err != nil {
		return err
	}

	if err := common.WriteUint32(w, u.Index); err != nil {
		return err
	}

	if err := u.Value.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (u *UTXO) Deserialize(r io.Reader) error {
	if err := u.TxId.Deserialize(r); err != nil {
		return err
	}

	var err error
	u.Index, err = common.ReadUint32(r)
	if err != nil {
		return err
	}

	if err := u.Value.Deserialize(r); err != nil {
		return err
	}

	return nil
}
