package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const RegisterIdentification = 0x09

const RegisterIdentificationVersion = 0x00

type RegisterIdentificationValue struct {
	DataHash common.Uint256
	Proof    string
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

func (a *PayloadRegisterIdentification) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	a.Serialize(buf, RegisterIdentificationVersion)
	return buf.Bytes()
}

func (a *PayloadRegisterIdentification) Serialize(w io.Writer, version byte) error {

	if err := common.WriteVarString(w, a.ID); err != nil {
		return errors.New("[RegisterIdentification], ID serialize failed.")
	}

	if err := common.WriteElement(w, a.Sign); err != nil {
		return errors.New("[RegisterIdentification], Sign serialize failed.")
	}

	if err := common.WriteVarUint(w, uint64(len(a.Contents))); err != nil {
		return errors.New("[RegisterIdentification], Content size serialize failed.")
	}

	for _, content := range a.Contents {
		if err := content.Serialize(w, version); err != nil {
			return err
		}
	}

	return nil
}

func (a *PayloadRegisterIdentification) Deserialize(r io.Reader, version byte) error {

	var err error
	a.ID, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentification], ID deserialize failed.")
	}

	if err := common.ReadElement(r, &a.Sign); err != nil {
		return errors.New("[RegisterIdentification], Sign deserialize failed.")
	}

	size, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New("[RegisterIdentification], Content size deserialize failed.")
	}

	a.Contents = make([]RegisterIdentificationContent, size)
	for i := uint64(0); i < size; i++ {
		content := RegisterIdentificationContent{}
		if err := content.Deserialize(r, version); err != nil {
			return err
		}
		a.Contents[i] = content
	}

	return nil
}

func (a *PayloadRegisterIdentification) GetData() []byte {
	return a.Data(RegisterIdentificationVersion)
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

	a.Values = make([]RegisterIdentificationValue, valueSize)
	for j := uint64(0); j < valueSize; j++ {
		value := RegisterIdentificationValue{}
		if err := value.Deserialize(r, version); err != nil {
			return err
		}
		a.Values[j] = value
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

	return nil
}
