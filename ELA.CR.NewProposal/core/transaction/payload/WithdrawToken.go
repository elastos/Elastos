package payload

import (
	"Elastos.ELA/common/serialization"
	"errors"
	"io"
)

const WithdrawTokenPayloadVersion byte = 0x00

type WithdrawToken struct {
	BlockHeight uint32
}

func (t *WithdrawToken) Data(version byte) []byte {
	return []byte{0}
}

func (t *WithdrawToken) Serialize(w io.Writer, version byte) error {
	if err := serialization.WriteUint32(w, t.BlockHeight); err != nil {
		return errors.New("[WithdrawToken], BlockHeight serialize failed.")
	}

	return nil
}

func (t *WithdrawToken) Deserialize(r io.Reader, version byte) error {
	height, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("[WithdrawToken], BlockHeight deserialize failed.")
	}
	t.BlockHeight = height

	return nil
}
