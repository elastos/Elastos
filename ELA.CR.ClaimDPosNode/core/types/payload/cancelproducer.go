package payload

import (
	"bytes"
	"errors"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type PayloadCancelProducer struct {
	PublicKey string
}

func (a *PayloadCancelProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadCancelProducer) Serialize(w io.Writer, version byte) error {
	err := WriteVarString(w, a.PublicKey)
	if err != nil {
		return errors.New("[PayloadCancelProducer], Serialize failed.")
	}
	return nil
}

func (a *PayloadCancelProducer) Deserialize(r io.Reader, version byte) error {
	pk, err := ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadCancelProducer], Deserialize failed.")
	}
	a.PublicKey = pk
	return err
}
