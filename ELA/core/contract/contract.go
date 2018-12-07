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

type PrefixType byte

const (
	PrefixStandard   PrefixType = 0x21
	PrefixMultiSig   PrefixType = 0x12
	PrefixCrossChain PrefixType = 0x4B
	PrefixPledge     PrefixType = 0x55
)

// Contract include the redeem script and hash prefix
type Contract struct {
	Code       []byte
	HashPrefix PrefixType
}

func (c *Contract) ToProgramHash() (*utilcom.Uint168, error) {
	code := c.Code
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
	programBytes := md160.Sum([]byte{byte(c.HashPrefix)})

	return utilcom.Uint168FromBytes(programBytes)
}

func (c *Contract) ToCodeHash() (*common.Uint160, error) {
	return common.ToCodeHash(c.Code)
}
