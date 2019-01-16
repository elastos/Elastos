package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const PayloadUpdateProducerVersion byte = 0x00

type PayloadUpdateProducer struct {
	OwnPublicKey  []byte
	NodePublicKey []byte
	NickName      string
	Url           string
	Location      uint64
	Address       string
	Signature     []byte
}

func (a *PayloadUpdateProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadUpdateProducer) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Signature serialize failed")
	}

	return nil
}

func (a *PayloadUpdateProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.OwnPublicKey)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Own public key serialize failed")
	}

	err = common.WriteVarBytes(w, a.NodePublicKey)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Node public key serialize failed")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], NickName serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Location serialize failed")
	}

	err = common.WriteVarString(w, a.Address)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Address serialize failed")
	}
	return nil
}

func (a *PayloadUpdateProducer) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	sig, err := common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *PayloadUpdateProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	ownPublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "own public key")
	if err != nil {
		return errors.New("[PayloadUpdateProducer], own public key deserialize failed")
	}

	nodePublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "node public key")
	if err != nil {
		return errors.New("[PayloadUpdateProducer], node public key deserialize failed")
	}

	nickName, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], NickName deserialize failed")
	}

	url, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Url deserialize failed")
	}

	location, err := common.ReadUint64(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Location deserialize failed")
	}

	addr, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], Address deserialize failed")
	}

	a.OwnPublicKey = ownPublicKey
	a.NodePublicKey = nodePublicKey
	a.NickName = nickName
	a.Url = url
	a.Location = location
	a.Address = addr

	return nil
}

func ConvertToRegisterProducerPayload(update *PayloadUpdateProducer) *PayloadRegisterProducer {
	return &PayloadRegisterProducer{
		OwnPublicKey:  update.OwnPublicKey,
		NodePublicKey: update.NodePublicKey,
		NickName:      update.NickName,
		Url:           update.Url,
		Location:      update.Location,
		Address:       update.Address,
	}
}

func ConvertToUpdateProducerPayload(register *PayloadRegisterProducer) *PayloadUpdateProducer {
	return &PayloadUpdateProducer{
		OwnPublicKey: register.OwnPublicKey,
		NickName:     register.NickName,
		Url:          register.Url,
		Location:     register.Location,
		Address:      register.Address,
	}
}
