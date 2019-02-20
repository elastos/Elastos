package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

type PayloadDeploy struct {
	Code        *FunctionCode
	Name        string
	CodeVersion string
	Author      string
	Email       string
	Description string
	ProgramHash common.Uint168
	Gas         common.Fixed64
}

func (dc *PayloadDeploy) Data(version byte) []byte  {
	buf := new(bytes.Buffer)
	err := dc.Serialize(buf, version)
	if err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (dc *PayloadDeploy) Serialize(w io.Writer, version byte) error {
	err := dc.Code.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, dc.Name)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, dc.CodeVersion)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, dc.Author)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, dc.Email)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, dc.Description)
	if err != nil {
		return err
	}

	err = dc.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	err = dc.Gas.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (dc *PayloadDeploy) Deserialize(r io.Reader, version byte) error {
	dc.Code = new (FunctionCode)
	err := dc.Code.Deserialize(r)
	if err != nil {
		return err
	}

	dc.Name, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	dc.CodeVersion, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	dc.Author, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	dc.Email, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	dc.Description, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	dc.ProgramHash = common.Uint168{}
	err = dc.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	gas := common.Fixed64(0)
	err = gas.Deserialize(r)
	if err != nil {
		return err
	}
	dc.Gas = gas

	return nil
}