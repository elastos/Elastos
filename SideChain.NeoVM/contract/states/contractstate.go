package states

import (
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

type ContractState struct {
	StateBase
	Code        *types.FunctionCode
	Name        string
	Version     string
	Author      string
	Email       string
	Description string
	ProgramHash common.Uint168
}

func NewContractState() *ContractState {
	Code := &types.FunctionCode{
		ReturnType: types.Void,
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

	f := new(types.FunctionCode)
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
	return types.IsMultiSigContract(contractState.Code.Code)
}

func (contractState *ContractState) IsSignatureCotract() bool {
	return types.IsSignatureCotract(contractState.Code.Code)
}
