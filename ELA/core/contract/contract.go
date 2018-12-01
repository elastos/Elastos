package contract

import (
	"crypto/sha256"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/vm"

	utilcom "github.com/elastos/Elastos.ELA.Utility/common"
	"golang.org/x/crypto/ripemd160"
)

type ContractType byte

const (
	Signature ContractType = iota
	MultiSig
	Custom
)

const (
	PrefixStandard   = 0x21
	PrefixMultisig   = 0x12
	PrefixCrossChain = 0x4B
	PrefixPledge     = 0x55
)

// Contract include the redeem script and hash prefix
type Contract struct {
	RedeemScript []byte
	HashPrefix   byte
}

func (c *Contract) IsStandard() bool {
	if len(c.RedeemScript) != 35 {
		return false
	}
	if c.RedeemScript[0] != 33 || c.RedeemScript[34] != byte(vm.CHECKSIG) {
		return false
	}
	return true
}

func (c *Contract) IsMultiSig() bool {
	var m int16 = 0
	var n int16 = 0
	i := 0

	if len(c.RedeemScript) < 37 {
		return false
	}
	if c.RedeemScript[i] > byte(vm.PUSH16) {
		return false
	}
	if c.RedeemScript[i] < byte(vm.PUSH1) && c.RedeemScript[i] != 1 && c.RedeemScript[i] != 2 {
		return false
	}

	switch c.RedeemScript[i] {
	case 1:
		i++
		m = int16(c.RedeemScript[i])
		i++
		break
	case 2:
		i++
		m = common.BytesToInt16(c.RedeemScript[i:])
		i += 2
		break
	default:
		m = int16(c.RedeemScript[i]) - 80
		i++
		break
	}

	if m < 1 || m > 1024 {
		return false
	}

	for c.RedeemScript[i] == 33 {
		i += 34
		if len(c.RedeemScript) <= i {
			return false
		}
		n++
	}
	if n < m || n > 1024 {
		return false
	}

	switch c.RedeemScript[i] {
	case 1:
		i++
		if n != int16(c.RedeemScript[i]) {
			return false
		}
		i++
		break
	case 2:
		i++
		if n != common.BytesToInt16(c.RedeemScript[i:]) {
			return false
		}
		i += 2
		break
	default:
		if n != (int16(c.RedeemScript[i]) - 80) {
			return false
		}
		i++
		break
	}

	if c.RedeemScript[i] != byte(vm.CHECKMULTISIG) {
		return false
	}
	i++
	if len(c.RedeemScript) != i {
		return false
	}

	return true
}

func (c *Contract) GetType() ContractType {
	if c.IsStandard() {
		return Signature
	}
	if c.IsMultiSig() {
		return MultiSig
	}
	return Custom
}

func (c *Contract) ToProgramHash() (*utilcom.Uint168, error) {
	code := c.RedeemScript
	if len(code) < 1 {
		return nil, errors.New("[ToProgramHash] failed, empty program code")
	}

	// Check code
	switch code[len(code)-1] {
	case vm.CHECKSIG:
		if len(code) != crypto.PublicKeyScriptLength {
			return nil, errors.New("[ToProgramHash] error, not a valid checksig script")
		}
	case vm.CHECKMULTISIG:
		if len(code) < crypto.MinMultiSignCodeLength || (len(code)-3)%(crypto.PublicKeyScriptLength-1) != 0 {
			return nil, errors.New("[ToProgramHash] error, not a valid multisig script")
		}
	case utilcom.CROSSCHAIN: // FIXME should not use this opcode
	default:
		return nil, errors.New("[ToProgramHash] error, unknown opcode")
	}

	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	programBytes := md160.Sum([]byte{c.HashPrefix})

	return utilcom.Uint168FromBytes(programBytes)
}
