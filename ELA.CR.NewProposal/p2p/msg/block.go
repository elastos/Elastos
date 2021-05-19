// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package msg

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Block implement p2p.Message interface.
var _ p2p.Message = (*Block)(nil)

type Block struct {
	common.Serializable
}

func NewBlock(block common.Serializable) *Block {
	return &Block{Serializable: block}
}

func (msg *Block) CMD() string {
	return p2p.CmdBlock
}

func (msg *Block) MaxLength() uint32 {
	return pact.MaxBlockContextSize + pact.MaxBlockHeaderSize
}
