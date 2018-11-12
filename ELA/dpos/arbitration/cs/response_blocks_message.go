package cs

import (
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/dpos/chain"
	"io"
)

type ResponseBlocksMessage struct {
	Command       string
	Blocks        []*chain.Block
	BlockConfirms []*chain.ProposalVoteSlot
}

func (msg *ResponseBlocksMessage) CMD() string {
	return msg.Command
}

func (msg *ResponseBlocksMessage) MaxLength() uint32 {
	//todo add max length
	return 0
}

func (msg *ResponseBlocksMessage) Serialize(w io.Writer) error {
	if err := common.WriteVarUint(w, uint64(len(msg.Blocks))); err != nil {
		return err
	}

	for _, v := range msg.Blocks {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(msg.BlockConfirms))); err != nil {
		return err
	}

	for _, v := range msg.BlockConfirms {
		if err := v.Serialize(w); err != nil {
			return err
		}
	}

	return nil
}

func (msg *ResponseBlocksMessage) Deserialize(r io.Reader) error {
	blockCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	msg.Blocks = make([]*chain.Block, 0)
	for i := uint64(0); i < blockCount; i++ {
		block := &chain.Block{}
		if err = block.Deserialize(r); err != nil {
			return err
		}
		msg.Blocks = append(msg.Blocks, block)
	}

	blockConfirmCount, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	msg.BlockConfirms = make([]*chain.ProposalVoteSlot, 0)
	for i := uint64(0); i < blockConfirmCount; i++ {
		blockConfirm := &chain.ProposalVoteSlot{}
		if err = blockConfirm.Deserialize(r); err != nil {
			return err
		}
		msg.BlockConfirms = append(msg.BlockConfirms, blockConfirm)
	}

	return nil
}
