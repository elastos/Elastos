package contract

import (
	"crypto/sha256"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"golang.org/x/crypto/ripemd160"
)

type PrefixType byte

const (
	PrefixStandard   PrefixType = 0x21
	PrefixMultiSig   PrefixType = 0x12
	PrefixCrossChain PrefixType = 0x4B
	PrefixDeposit    PrefixType = 0x1F
)

// Contract include the redeem script and hash prefix
type Contract struct {
	Code       []byte
	HashPrefix PrefixType
}

func (c *Contract) ToProgramHash() (*common.Uint168, error) {
	code := c.Code
	if len(code) < 1 {
		return nil, errors.New("[ToProgramHash] failed, empty program code")
	}

	hash := sha256.Sum256(code)
	md160 := ripemd160.New()
	md160.Write(hash[:])
	programBytes := md160.Sum([]byte{byte(c.HashPrefix)})

	return common.Uint168FromBytes(programBytes)
}

func (c *Contract) ToCodeHash() *common.Uint160 {
	return common.ToCodeHash(c.Code)
}
