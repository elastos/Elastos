package contract

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
)

//Contract address is the hash of contract program .
//which be used to control asset or indicate the smart contract address �?

//Contract include the program codes with parameters which can be executed on specific evnrioment
type Contract struct {

	//the contract program code,which will be run on VM or specific envrionment
	Code []byte

	//the Contract Parameter type list
	// describe the number of contract program parameters and the parameter type
	Parameters []ContractParameterType

	//The program hash as contract address
	ProgramHash common.Uint168

	//owner's pubkey hash indicate the owner of contract
	OwnerPubkeyHash common.Uint168
}


func (c *Contract) Deserialize(r io.Reader) error {
	c.OwnerPubkeyHash.Deserialize(r)

	p, err := common.ReadVarBytes(r, avm.MaxParameterSize, "Contract Deserialize Parameters")
	if err != nil {
		return err
	}
	c.Parameters = ByteToContractParameterType(p)

	c.Code, err = common.ReadVarBytes(r, avm.MaxItemSize, "Contract Deserialize Code")
	if err != nil {
		return err
	}

	return nil
}

func (c *Contract) Serialize(w io.Writer) error {
	err := c.OwnerPubkeyHash.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, ContractParameterTypeToByte(c.Parameters))
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, c.Code)
	if err != nil {
		return err
	}

	return nil
}

func (c *Contract) ToArray() []byte {
	w := new(bytes.Buffer)
	c.Serialize(w)

	return w.Bytes()
}


func ContractParameterTypeToByte(c []ContractParameterType) []byte {
	size := len(c)
	b := make([]byte, size)
	for i := 0; i < size; i++ {
		b[i] = byte(c[i])
	}
	return b
}

func ByteToContractParameterType(b []byte) []ContractParameterType {
	size := len(b)
	c := make([]ContractParameterType, size)
	for i := 0; i < size; i++ {
		c[i] = ContractParameterType(b[i])
	}

	return c
}