package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/common"

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


func IsMultiSigContract(script []byte) bool {
	m := 0
	n := 0
	i := 0

	if len(script) < 37 {
		return false
	}
	if script[i] > avm.PUSH16 {
		return false
	}

	if script[i] < avm.PUSH1 && script[i] != 1 && script[i] != 2 {
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

	if script[i] != avm.CHECKMULTISIG {
		i++
		return false
	}
	i++
	if len(script) != i {
		return false
	}

	return true
}

func IsSignatureCotract(script []byte) bool {
	if len(script) != 35 {
		return false
	}
	if script[0] != 33 || script[34] != avm.CHECKSIG {
		return false
	}
	return true
}