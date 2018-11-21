package program

import (
	"errors"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	// MaxProgramCodeSize is the maximum allowed length of program code.
	MaxProgramCodeSize = 10000

	// MaxProgramParamSize is the maximum allowed length of program parameter.
	MaxProgramParamSize = MaxProgramCodeSize * 2
)

type Program struct {
	//the contract program code,which will be run on VM or specific environment
	Code []byte

	//the program code's parameter
	Parameter []byte
}

func (p Program) String() string {
	return "Program: {\n\t\t" +
		"Code: " + BytesToHexString(p.Code) + "\n\t\t" +
		"Parameter: " + BytesToHexString(p.Parameter) + "\n\t\t" +
		"}"
}

//Serialize the Program
func (p *Program) Serialize(w io.Writer) error {
	if err := WriteVarBytes(w, p.Parameter); err != nil {
		return errors.New("Execute Program Serialize Parameter failed.")
	}

	if err := WriteVarBytes(w, p.Code); err != nil {
		return errors.New("Execute Program Serialize Code failed.")
	}

	return nil
}

//Deserialize the Program
func (p *Program) Deserialize(w io.Reader) error {
	parameter, err := ReadVarBytes(w, MaxProgramParamSize,
		"program parameter")
	if err != nil {
		return errors.New("Execute Program Deserialize Parameter failed.")
	}
	p.Parameter = parameter

	code, err := ReadVarBytes(w, MaxProgramCodeSize,
		"program code")
	if err != nil {
		return errors.New("Execute Program Deserialize Code failed.")
	}
	p.Code = code

	return nil
}
