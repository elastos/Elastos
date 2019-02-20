package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const PayloadCancelProducerVersion byte = 0x00

type PayloadCancelProducer struct {
	OwnerPublicKey []byte
	Signature      []byte
}

func (a *PayloadCancelProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadCancelProducer) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[PayloadCancelProducer], signature serialize failed")
	}

	return nil
}

func (a *PayloadCancelProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.OwnerPublicKey)
	if err != nil {
		return errors.New("[PayloadCancelProducer], serialize failed")
	}
	return nil
}

func (a *PayloadCancelProducer) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	sig, err := common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[PayloadCancelProducer], signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *PayloadCancelProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	pk, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
	if err != nil {
		return errors.New("[PayloadCancelProducer], deserialize failed")
	}
	a.OwnerPublicKey = pk
	return err
}
