package contract

import (
	"github.com/elastos/Elastos.ELA/common"
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
	Code   []byte
	Prefix PrefixType
}

func (c *Contract) ToProgramHash() *common.Uint168 {
	return common.ToProgramHash(byte(c.Prefix), c.Code)
}

func (c *Contract) ToCodeHash() *common.Uint160 {
	return common.ToCodeHash(c.Code)
}
