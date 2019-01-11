package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const ProducerInfoVersion byte = 0x00

type ProducerInfo struct {
	PublicKey []byte
	NickName  string
	Url       string
	Location  uint64
	Address   string
	Signature []byte
}

func (a *ProducerInfo) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *ProducerInfo) Serialize(w io.Writer, version byte) error {
	err := a.SerializeUnsigned(w, version)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, a.Signature)
	if err != nil {
		return errors.New("[ProducerInfo], Signature serialize failed")
	}

	return nil
}

func (a *ProducerInfo) SerializeUnsigned(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.PublicKey)
	if err != nil {
		return errors.New("[ProducerInfo], PublicKey serialize failed")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[ProducerInfo], NickName serialize failed")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[ProducerInfo], Url serialize failed")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[ProducerInfo], Location serialize failed")
	}

	err = common.WriteVarString(w, a.Address)
	if err != nil {
		return errors.New("[ProducerInfo], Address serialize failed")
	}
	return nil
}

func (a *ProducerInfo) Deserialize(r io.Reader, version byte) error {
	err := a.DeserializeUnsigned(r, version)
	if err != nil {
		return err
	}
	sig, err := common.ReadVarBytes(r, crypto.SignatureLength, "signature")
	if err != nil {
		return errors.New("[ProducerInfo], Signature deserialize failed")
	}

	a.Signature = sig

	return nil
}

func (a *ProducerInfo) DeserializeUnsigned(r io.Reader, version byte) error {
	publicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
	if err != nil {
		return errors.New("[ProducerInfo], PublicKey deserialize failed")
	}

	nickName, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], NickName deserialize failed")
	}

	url, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], Url deserialize failed")
	}

	location, err := common.ReadUint64(r)
	if err != nil {
		return errors.New("[ProducerInfo], Location deserialize failed")
	}

	addr, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[ProducerInfo], Address deserialize failed")
	}

	a.PublicKey = publicKey
	a.NickName = nickName
	a.Url = url
	a.Location = location
	a.Address = addr

	return nil
}
