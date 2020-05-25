// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure GetBlockByHash implement p2p.Message interface.
var _ p2p.Message = (*GetBlock)(nil)

type GetBlock struct {
	Inventory
}

func NewGetBlock(blockHash common.Uint256) *GetBlock {
	getBlock := new(GetBlock)
	getBlock.BlockHash = blockHash
	return getBlock
}

func (msg *GetBlock) CMD() string {
	return CmdGetBlock
}
