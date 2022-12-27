package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	ActivateProducerVersion byte = 0x00
)

type ActivateProducer struct {
	NodePublicKey []byte
	Signature     []byte
}

func (a *ActivateProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *ActivateProducer) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[ActivateProducer], signature serialize failed")
	}

	return nil
}

func (a *ActivateProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.NodePublicKey)
	if err != nil {
		return errors.New("[ActivateProducer], write owner public key failed")
	}

	return nil
}

func (a *ActivateProducer) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}

	a.Signature, err = common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[ActivateProducer], signature deserialize failed")
	}

	return nil
}

func (a *ActivateProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	var err error
	a.NodePublicKey, err = common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
	if err != nil {
		return errors.New("[ActivateProducer], read owner public key failed")
	}

	return err
}
