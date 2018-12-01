package types

import (
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type BlockConfirm struct {
	BlockFlag   bool
	Block       *Block
	ConfirmFlag bool
	Confirm     *DPosProposalVoteSlot
}

func (b *BlockConfirm) Serialize(w io.Writer) error {
	var blockFlag uint8
	if b.BlockFlag {
		blockFlag = uint8(1)
	}
	err := common.WriteUint8(w, blockFlag)
	if err != nil {
		return errors.New("Block flag serialize failed," + err.Error())
	}
	if b.BlockFlag {
		if err := b.Block.Serialize(w); err != nil {
			return errors.New("Block serialize failed," + err.Error())
		}
	}
	var confirmFlag uint8
	if b.ConfirmFlag {
		confirmFlag = uint8(1)
	}
	err = common.WriteUint8(w, confirmFlag)
	if err != nil {
		return errors.New("Confirm flag serialize failed," + err.Error())
	}
	if b.ConfirmFlag {
		if err := b.Confirm.Serialize(w); err != nil {
			return errors.New("Confirm serialize failed," + err.Error())
		}
	}
	return nil
}

func (b *BlockConfirm) Deserialize(r io.Reader) error {
	blockFlag, err := common.ReadUint8(r)
	if err != nil {
		return errors.New("Block flag dserialize failed," + err.Error())
	}
	if blockFlag == 1 {
		b.BlockFlag = true
	}
	if b.BlockFlag {
		b.Block = new(Block)
		if err := b.Block.Deserialize(r); err != nil {
			return errors.New("Block dserialize failed," + err.Error())
		}
	}

	confirmFlag, err := common.ReadUint8(r)
	if err != nil {
		return errors.New("Confirm flag dserialize failed," + err.Error())
	}
	if confirmFlag == 1 {
		b.ConfirmFlag = true
	}
	if b.ConfirmFlag {
		b.Confirm = new(DPosProposalVoteSlot)
		if err := b.Confirm.Deserialize(r); err != nil {
			return errors.New("Confirm serialize failed," + err.Error())
		}
	}

	return nil
}
