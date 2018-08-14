package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

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

		if err := common.WriteVarString(w, content.Path); err != nil {
			return errors.New("[RegisterIdentification], path serialize failed.")
		}

		if err := common.WriteVarUint(w, uint64(len(content.Values))); err != nil {
			return errors.New("[RegisterIdentification], Values size serialize failed.")
		}

		for _, value := range content.Values {
			if err := common.WriteElement(w, value.DataHash); err != nil {
				return errors.New("[RegisterIdentification], DataHash serialize failed.")
			}

			if err := common.WriteVarString(w, value.Proof); err != nil {
				return errors.New("[RegisterIdentification], Proof serialize failed.")
			}
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

		content.Path, err = common.ReadVarString(r)
		if err != nil {
			return errors.New("[RegisterIdentification], path deserialize failed.")
		}

		valueSize, err := common.ReadVarUint(r, 0)
		if err != nil {
			return errors.New("[RegisterIdentification], Values size deserialize failed.")
		}

		content.Values = make([]RegisterIdentificationValue, valueSize)
		for j := uint64(0); j < valueSize; j++ {
			value := RegisterIdentificationValue{}
			if err := common.ReadElement(r, &value.DataHash); err != nil {
				return errors.New("[RegisterIdentification], DataHash deserialize failed.")
			}

			value.Proof, err = common.ReadVarString(r)
			if err != nil {
				return errors.New("[RegisterIdentification], Proof deserialize failed.")
			}
		}

		a.Contents[i] = content
	}

	return nil
}

// VM IDataContainer interface
func (a *PayloadRegisterIdentification) GetData() []byte {
	return a.Data(RegisterIdentificationVersion)
}
