package program

import (
	"io"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/serialize"
)

type Program struct {
	//the contract program code,which will be run on VM or specific envrionment
	Code []byte

	//the program code's parameter
	Parameter []byte
}

func (self Program) String() string {
	return "Program: {\n\t\t" +
		"Code: " + common.BytesToHexString(self.Code) + "\n\t\t" +
		"Parameter: " + common.BytesToHexString(self.Parameter) + "\n\t\t" +
		"}"
}

//Serialize the Program
func (p *Program) Serialize(w io.Writer) error {
	if err := serialize.WriteVarBytes(w, p.Parameter); err != nil {
		return errors.New("Execute Program Serialize Parameter failed.")
	}

	if err := serialize.WriteVarBytes(w, p.Code); err != nil {
		return errors.New("Execute Program Serialize Code failed.")
	}

	return nil
}

//Deserialize the Program
func (p *Program) Deserialize(w io.Reader) error {
	parameter, err := serialize.ReadVarBytes(w)
	if err != nil {
		return errors.New("Execute Program Deserialize Parameter failed.")
	}
	p.Parameter = parameter

	code, err := serialize.ReadVarBytes(w)
	if err != nil {
		return errors.New("Execute Program Deserialize Code failed.")
	}
	p.Code = code

	return nil
}
