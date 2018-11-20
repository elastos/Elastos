package states

import (
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.SideChain/vm"
)

type ContractState struct {
	StateBase
	Code        *contract.FunctionCode
	Name        string
	Version     string
	Author      string
	Email       string
	Description string
	ProgramHash common.Uint168
}

func NewContractState() *ContractState {
	Code := &contract.FunctionCode{
		ReturnType: contract.Void,
	}
	return &ContractState{
		Code : Code,
	}
}

func (contractState *ContractState) Serialize(w io.Writer) error {
	err := contractState.StateBase.Serialize(w)
	if err != nil {
		return err
	}

	err = contractState.Code.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, contractState.Name)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, contractState.Version)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, contractState.Author)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, contractState.Email)
	if err != nil {
		return err
	}

	err = common.WriteVarString(w, contractState.Description)
	if err != nil {
		return err
	}

	err = contractState.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (contractState *ContractState) Deserialize(r io.Reader) error {
	stateBase := StateBase{}
	err := stateBase.Deserialize(r)
	if err != nil {
		return err
	}
	contractState.StateBase = stateBase

	f := new(contract.FunctionCode)
	err = f.Deserialize(r)
	if err != nil {
		return err
	}
	contractState.Code = f

	contractState.Name, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	contractState.Version, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	contractState.Author, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	contractState.Email, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	contractState.Description, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	hash := common.Uint168{}
	err = hash.Deserialize(r)
	if err != nil {
		return err
	}
	contractState.ProgramHash = hash

	return nil
}

func (contractState *ContractState) Bytes() []byte {
	b := new(bytes.Buffer)
	err := contractState.Serialize(b)
	if err != nil {
		return nil
	}
	return b.Bytes()
}

func (contractState *ContractState) IsMultiSigContract() bool {
	script := contractState.Code.Code
	m := 0
	n := 0
	i := 0

	if len(script) < 37 {
		return false
	}
	if script[i] > vm.PUSH16 {
		return false;
	}

	if script[i] < vm.PUSH1 && script[i] != 1 && script[i] != 2 {
		return false
	}
	switch script[i] {
	case 1:
		i++
		m = int(script[i])
		i++
	case 2:
		i++
		m = int(script[i])
		i += 2
	default:
		m = int(script[i]) - 80
		i++
	}
	if m < 1 || m > 1024 {
		return false
	}
	for script[i] == 33 {
		i += 34
		if len(script) <= i {
			return false
		}
		n++
	}
	if n < m || n > 1024 {
		return false
	}
	switch script[i] {
	case 1:
		i++
		if n != int(script[i]) {
			return false
		}
		i++
	case 2:
		if len(script) < i + 3 {
			return false
		} else {
			i++
			if n != int(script[i]) {
				return false
			}
		}
		i += 2
	default:
		if n != int(script[i]) - 80 {
			i++
			return false
		}
		i++
	}

	if script[i] != vm.CHECKMULTISIG {
		i++
		return false
	}
	i++
	if len(script) != i {
		return false
	}

	return true
}

func (contractState *ContractState) IsSignatureCotract() bool {
	script := contractState.Code.Code

	if len(script) != 35 {
		return false
	}
	if script[0] != 33 || script[34] != vm.CHECKSIG {
		return false
	}
	return true
}
