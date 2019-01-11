package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const CancelProducerVersion byte = 0x00

type CancelProducer struct {
	PublicKey []byte
	Signature []byte
}

func (a *CancelProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *CancelProducer) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[CancelProducer], Signature serialize failed")
	}

	return nil
}

func (a *CancelProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.PublicKey)
	if err != nil {
		return errors.New("[CancelProducer], Serialize failed")
	}
	return nil
}

func (a *CancelProducer) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	sig, err := common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[CancelProducer], Signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *CancelProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	pk, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
	if err != nil {
		return errors.New("[CancelProducer], Deserialize failed")
	}
	a.PublicKey = pk
	return err
}
