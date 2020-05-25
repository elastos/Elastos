// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/elastos/Elastos.ELA/common"
)

//todo move to config
const DefaultResponseBlocksMessageDataSize = 8000000 * 10

type ResponseBlocks struct {
	Command       string
	BlockConfirms []*types.DposBlock
}

func (m *ResponseBlocks) CMD() string {
	return CmdResponseBlocks
}

func (m *ResponseBlocks) MaxLength() uint32 {
	return DefaultResponseBlocksMessageDataSize
}

func (m *ResponseBlocks) Serialize(w io.Writer) error {
	if err := common.WriteVarUint(w, uint64(len(m.BlockConfirms))); err != nil {
		return err
	}

	for _, v := range m.BlockConfirms {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (m *ResponseBlocks) Deserialize(r io.Reader) error {
	blockConfirmCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	m.BlockConfirms = make([]*types.DposBlock, 0)
	for i := uint64(0); i < blockConfirmCount; i++ {
		dposBlock := &types.DposBlock{}
		if err = dposBlock.Deserialize(r); err != nil {
			return err
		}
		m.BlockConfirms = append(m.BlockConfirms, dposBlock)
	}

	return nil
}
