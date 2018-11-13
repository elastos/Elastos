package cs

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/core"
)

type ResponseBlocksMessage struct {
	Command       string
	Blocks        []*core.Block
	BlockConfirms []*msg.DPosProposalVoteSlot
}

func (m *ResponseBlocksMessage) CMD() string {
	return m.Command
}

func (m *ResponseBlocksMessage) MaxLength() uint32 {
	//todo add max length
	return 0
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

	m.BlockConfirms = make([]*msg.DPosProposalVoteSlot, 0)
	for i := uint64(0); i < blockConfirmCount; i++ {
		blockConfirm := &msg.DPosProposalVoteSlot{}
		if err = blockConfirm.Deserialize(r); err != nil {
			return err
		}
		m.BlockConfirms = append(m.BlockConfirms, blockConfirm)
	}

	return nil
}
