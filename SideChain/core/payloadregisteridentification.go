package core

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const RegisterIdentificationVersion = 0x00

type PayloadRegisterIdentification struct {
	ID       string
	Path     string
	DataHash common.Uint256
	Proof    string
	Sign     []byte
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

	if err := common.WriteVarString(w, a.Path); err != nil {
		return errors.New("[RegisterIdentification], path serialize failed.")
	}

	if err := common.WriteElement(w, a.DataHash); err != nil {
		return errors.New("[RegisterIdentification], DataHash serialize failed.")
	}

	if err := common.WriteVarString(w, a.Proof); err != nil {
		return errors.New("[RegisterIdentification], Proof serialize failed.")
	}

	if err := common.WriteElement(w, a.Sign); err != nil {
		return errors.New("[RegisterIdentification], Sign serialize failed.")
	}
	return nil
}

func (a *PayloadRegisterIdentification) Deserialize(r io.Reader, version byte) error {

	var err error
	a.ID, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentification], ID deserialize failed.")
	}

	a.Path, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentification], path deserialize failed.")
	}

	if err := common.ReadElement(r, &a.DataHash); err != nil {
		return errors.New("[RegisterIdentification], DataHash deserialize failed.")
	}

	a.Proof, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[RegisterIdentification], Proof deserialize failed.")
	}

	if err := common.ReadElement(r, &a.Sign); err != nil {
		return errors.New("[RegisterIdentification], Sign deserialize failed.")
	}

	return nil
}

// VM IDataContainer interface
func (a *PayloadRegisterIdentification) GetData() []byte {
	return a.Data(RegisterIdentificationVersion)
}
