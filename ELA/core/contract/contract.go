// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

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
	PrefixCRDID      PrefixType = 0x67
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
