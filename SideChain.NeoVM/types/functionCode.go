package types

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/params"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract"
)

type FunctionCode struct {
	// Contract Code
	Code []byte

	// Contract parameter type list
	ParameterTypes []contract.ContractParameterType

	// Contract return type list
	ReturnType contract.ContractParameterType

	codeHash common.Uint168
}


// method of SerializableData
func (fc *FunctionCode) Serialize(w io.Writer) error {
	err := common.WriteVarBytes(w, fc.Code)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, contract.ContractParameterTypeToByte(fc.ParameterTypes))
	if err != nil {
		return err
	}

	err = common.WriteUint8(w, uint8(fc.ReturnType))
	if err != nil {
		return err
	}

	return nil
}

// method of SerializableData
func (fc *FunctionCode) Deserialize(r io.Reader) error {
	code, err := common.ReadVarBytes(r, avm.MaxItemSize, "FunctionCode Deserialize Code")
	if err != nil {
		return err
	}
	fc.Code = code

	parameterTypes, err := common.ReadVarBytes(r, avm.MaxParameterSize, "FunctionCode Deserialize parameterTypes")
	if err != nil {
		return err
	}
	fc.ParameterTypes = contract.ByteToContractParameterType(parameterTypes)

	returnType, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	fc.ReturnType = contract.ContractParameterType(returnType)

	return nil
}

// Get code
func (fc *FunctionCode) GetCode() []byte {
	return fc.Code
}

// Get the list of parameter value
func (fc *FunctionCode) GetParameterTypes() []contract.ContractParameterType {
	return fc.ParameterTypes
}

// Get the list of return value
func (fc *FunctionCode) GetReturnType() contract.ContractParameterType {
	return fc.ReturnType
}

// Get the hash of the smart contract
func (fc *FunctionCode) CodeHash() common.Uint168 {
	zeroHash := common.Uint168{}
	if fc.codeHash == zeroHash {
		hash, err := params.ToCodeHash(fc.Code)
		if err != nil {
			return zeroHash
		}
		fc.codeHash = *hash
	}
	return fc.codeHash
}