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
	Proof    []byte
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

	if err := common.WriteElements(w, a.DataHash, a.Proof, a.Sign); err != nil {
		return errors.New("[RegisterIdentification], others serialize failed.")
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

	if err := common.ReadElements(r, &a.DataHash, &a.Proof, &a.Sign); err != nil {
		return errors.New("[RegisterIdentification], others deserialize failed.")
	}

	return nil
}

// VM IDataContainer interface
func (a *PayloadRegisterIdentification) GetData() []byte {
	return a.Data(RegisterIdentificationVersion)
}
