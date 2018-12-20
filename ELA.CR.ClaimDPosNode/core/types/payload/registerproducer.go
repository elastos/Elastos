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
	PublicKey []byte
	NickName  string
	Url       string
	Location  uint64
	Address   string
}

func (a *PayloadRegisterProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadRegisterProducer) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarBytes(w, a.PublicKey)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], PublicKey serialize failed.")
	}

	err = common.WriteVarString(w, a.NickName)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NickName serialize failed.")
	}

	err = common.WriteVarString(w, a.Url)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Url serialize failed.")
	}

	err = common.WriteUint64(w, a.Location)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Location serialize failed.")
	}

	err = common.WriteVarString(w, a.Address)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Address serialize failed.")
	}
	return nil
}

func (a *PayloadRegisterProducer) Deserialize(r io.Reader, version byte) error {
	publicKey, err := common.ReadVarBytes(r, crypto.NegativeBigLength, "public key")
	if err != nil {
		return errors.New("[PayloadRegisterProducer], PublicKey deserialize failed.")
	}

	nickName, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], NickName deserialize failed.")
	}

	url, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Url deserialize failed.")
	}

	location, err := common.ReadUint64(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Location deserialize failed.")
	}

	addr, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadRegisterProducer], Address deserialize failed.")
	}

	a.PublicKey = publicKey
	a.NickName = nickName
	a.Url = url
	a.Location = location
	a.Address = addr

	return nil
}
