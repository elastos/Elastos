package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const PayloadRegisterProducerVersion byte = 0x00

type PayloadRegisterProducer struct {
	OwnerPublicKey []byte
	NodePublicKey  []byte
	NickName       string
	Url            string
	Location       uint64
	Address        string
	Signature      []byte
}

func (a *PayloadRegisterProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadRegisterProducer) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Signature serialize failed")
	}

	return nil
}

func (a *PayloadRegisterProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.OwnerPublicKey)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], OwnerPublicKey serialize failed")
	}

	err = common.WriteVarBytes(w, a.NodePublicKey)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NodePublicKey serialize failed")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NickName serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Location serialize failed")
	}

	err = common.WriteVarString(w, a.Address)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Address serialize failed")
	}
	return nil
}

func (a *PayloadRegisterProducer) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	sig, err := common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *PayloadRegisterProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	ownerPublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "own public key")
	if err != nil {
		return errors.New("[PayloadRegisterProducer], OwnerPublicKey deserialize failed")
	}

	nodePublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "node public key")
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NodePublicKey deserialize failed")
	}

	nickName, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NickName deserialize failed")
	}

	url, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Url deserialize failed")
	}

	location, err := common.ReadUint64(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Location deserialize failed")
	}

	addr, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Address deserialize failed")
	}

	a.OwnerPublicKey = ownerPublicKey
	a.NodePublicKey = nodePublicKey
	a.NickName = nickName
	a.Url = url
	a.Location = location
	a.Address = addr

	return nil
}
