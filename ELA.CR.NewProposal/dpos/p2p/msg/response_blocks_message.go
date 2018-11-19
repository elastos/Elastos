package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

//todo move to config
const DefaultResponseBlocksMessageDataSize = 8000000 * 10

type ResponseBlocksMessage struct {
	Command       string
	Blocks        []*core.Block
	BlockConfirms []*core.DPosProposalVoteSlot
}

func (m *ResponseBlocksMessage) CMD() string {
	return ResponseBlocks
}

func (m *ResponseBlocksMessage) MaxLength() uint32 {
	return DefaultResponseBlocksMessageDataSize
}

func (m *ResponseBlocksMessage) Serialize(w io.Writer) error {
	if err := common.WriteVarUint(w, uint64(len(m.Blocks))); err != nil {
		return err
	}

	for _, v := range m.Blocks {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

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

func (m *ResponseBlocksMessage) Deserialize(r io.Reader) error {
	blockCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	m.Blocks = make([]*core.Block, 0)
	for i := uint64(0); i < blockCount; i++ {
		block := &core.Block{}
		if err = block.Deserialize(r); err != nil {
			return err
		}
		m.Blocks = append(m.Blocks, block)
	}

	blockConfirmCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	m.BlockConfirms = make([]*core.DPosProposalVoteSlot, 0)
	for i := uint64(0); i < blockConfirmCount; i++ {
		blockConfirm := &core.DPosProposalVoteSlot{}
		if err = blockConfirm.Deserialize(r); err != nil {
			return err
		}
		m.BlockConfirms = append(m.BlockConfirms, blockConfirm)
	}

	return nil
}
