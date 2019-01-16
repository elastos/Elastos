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
	OwnerPublicKey []byte
	NodePublicKey  []byte
	NickName       string
	Url            string
	Location       uint64
	Address        string
	Signature      []byte
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
		return errors.New("[PayloadUpdateProducer], signature serialize failed")
	}

	return nil
}

func (a *PayloadUpdateProducer) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.OwnerPublicKey)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], owner public key serialize failed")
	}

	err = common.WriteVarBytes(w, a.NodePublicKey)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], node public key serialize failed")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], nickname serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], location serialize failed")
	}

	err = common.WriteVarString(w, a.Address)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], address serialize failed")
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
		return errors.New("[PayloadUpdateProducer], signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *PayloadUpdateProducer) DeserializeUnsigned(r io.Reader, version byte) error {
	ownerPublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "own public key")
	if err != nil {
		return errors.New("[PayloadUpdateProducer], owner public key deserialize failed")
	}

	nodePublicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "node public key")
	if err != nil {
		return errors.New("[PayloadUpdateProducer], node public key deserialize failed")
	}

	nickName, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], nickname deserialize failed")
	}

	url, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], url deserialize failed")
	}

	location, err := common.ReadUint64(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], location deserialize failed")
	}

	addr, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadUpdateProducer], address deserialize failed")
	}

	a.OwnerPublicKey = ownerPublicKey
	a.NodePublicKey = nodePublicKey
	a.NickName = nickName
	a.Url = url
	a.Location = location
	a.Address = addr

	return nil
}

func ConvertToRegisterProducerPayload(update *PayloadUpdateProducer) *PayloadRegisterProducer {
	return &PayloadRegisterProducer{
		OwnerPublicKey: update.OwnerPublicKey,
		NodePublicKey:  update.NodePublicKey,
		NickName:       update.NickName,
		Url:            update.Url,
		Location:       update.Location,
		Address:        update.Address,
	}
}
