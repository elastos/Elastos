package contract

import (
	"bytes"
	"io"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/serialize"
)

//Contract address is the hash of contract program .
//which be used to control asset or indicate the smart contract address

//Contract include the program codes with parameters which can be executed on specific evnrioment
type Contract struct {
	//the contract program code,which will be run on VM or specific envrionment
	Code []byte

	//the Contract Parameter type list
	// describe the number of contract program parameters and the parameter type
	Parameters []ContractParameterType

	//The program hash as contract address
	ProgramHash Uint168

	//owner's pubkey hash indicate the owner of contract
	OwnerPubkeyHash Uint168
}

func (c *Contract) Deserialize(r io.Reader) error {
	err := c.OwnerPubkeyHash.Deserialize(r)
	if err != nil {
		return err
	}

	p, err := serialize.ReadVarBytes(r)
	if err != nil {
		return err
	}
	c.Parameters = ByteToContractParameterType(p)

	c.Code, err = serialize.ReadVarBytes(r)
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

	err = serialize.WriteVarBytes(w, ContractParameterTypeToByte(c.Parameters))
	if err != nil {
		return err
	}

	err = serialize.WriteVarBytes(w, c.Code)
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
	b := make([]byte, len(c))

	for i := 0; i < len(c); i++ {
		b[i] = byte(c[i])
	}

	return b
}

func ByteToContractParameterType(b []byte) []ContractParameterType {
	c := make([]ContractParameterType, len(b))

	for i := 0; i < len(b); i++ {
		c[i] = ContractParameterType(b[i])
	}

	return c
}
