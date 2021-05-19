package types

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const RegisterIdentification = 0x09
const RegisterIdentificationVersion = 0x00
const MaxSignDataSize = 1000

type RegisterIdentificationValue struct {
	DataHash common.Uint256
	Proof    string
	Info     string
}

type RegisterIdentificationContent struct {
	Path   string
	Values []RegisterIdentificationValue
}

type PayloadRegisterIdentification struct {
	ID       string
	Sign     []byte
	Contents []RegisterIdentificationContent
}

func (p *PayloadRegisterIdentification) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	p.Serialize(buf, RegisterIdentificationVersion)
	return buf.Bytes()
}

func (p *PayloadRegisterIdentification) Serialize(w io.Writer, version byte) error {

	if err := common.WriteVarString(w, p.ID); err != nil {
		return errors.New("[RegisterIdentification], ID serialize failed.")
	}

	if err := common.WriteVarBytes(w, p.Sign); err != nil {
		return errors.New("[RegisterIdentification], Sign serialize failed.")
	}

	if err := common.WriteVarUint(w, uint64(len(p.Contents))); err != nil {
		return errors.New("[RegisterIdentification], Content size serialize failed.")
	}

	for _, content := range p.Contents {
		if err := content.Serialize(w, version); err != nil {
			return err
		}
	}

	return nil
}

func (p *PayloadRegisterIdentification) Deserialize(r io.Reader, version byte) error {

	var err error
	p.ID, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentification], ID deserialize failed.")
	}

	sign, err := common.ReadVarBytes(r, MaxSignDataSize, "RegisterIdentification sign")
	if err != nil {
		return errors.New("[RegisterIdentification], Sign deserialize failed.")
	}
	p.Sign = sign

	size, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("[RegisterIdentification], Content size deserialize failed.")
	}

	p.Contents = make([]RegisterIdentificationContent, size)
	for i := uint64(0); i < size; i++ {
		content := RegisterIdentificationContent{}
		if err := content.Deserialize(r, version); err != nil {
			return err
		}
		p.Contents[i] = content
	}

	return nil
}

func (p *PayloadRegisterIdentification) GetData() []byte {
	return p.Data(RegisterIdentificationVersion)
}

func (a *RegisterIdentificationContent) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, a.Path); err != nil {
		return errors.New("[RegisterIdentificationContent], path serialize failed.")
	}

	if err := common.WriteVarUint(w, uint64(len(a.Values))); err != nil {
		return errors.New("[RegisterIdentificationContent], Values size serialize failed.")
	}

	for _, value := range a.Values {
		if err := value.Serialize(w, version); err != nil {
			return err
		}
	}

	return nil
}

func (a *RegisterIdentificationContent) Deserialize(r io.Reader, version byte) error {
	path, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentificationContent], path deserialize failed.")
	}
	a.Path = path

	valueSize, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("[RegisterIdentificationContent], Values size deserialize failed.")
	}

	a.Values = make([]RegisterIdentificationValue, 0)
	for j := uint64(0); j < valueSize; j++ {
		value := RegisterIdentificationValue{}
		if err := value.Deserialize(r, version); err != nil {
			return err
		}
		a.Values = append(a.Values, value)
	}

	return nil
}

func (a *RegisterIdentificationValue) Serialize(w io.Writer, version byte) error {
	if err := a.DataHash.Serialize(w); err != nil {
		return errors.New("[RegisterIdentificationValue], DataHash serialize failed.")
	}

	if err := common.WriteVarString(w, a.Proof); err != nil {
		return errors.New("[RegisterIdentificationValue], Proof serialize failed.")
	}

	if err := common.WriteVarString(w, a.Info); err != nil {
		return errors.New("[RegisterIdentificationValue], Info serialize failed.")
	}

	return nil
}

func (a *RegisterIdentificationValue) Deserialize(r io.Reader, version byte) error {
	if err := a.DataHash.Deserialize(r); err != nil {
		return errors.New("[RegisterIdentificationValue], DataHash deserialize failed.")
	}

	proof, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentificationValue], Proof deserialize failed.")
	}
	a.Proof = proof

	info, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentificationValue], Info deserialize failed.")
	}
	a.Info = info

	return nil
}
