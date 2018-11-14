package blockchain

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/core"
)

func CheckConfirm(confirm *core.DPosProposalVoteSlot) error {
	// TODO need to implement
	fmt.Println("CheckConfirm Tracer ...")
	return nil
}

func CheckBlockWithConfirmation(block *core.Block, confirm *core.DPosProposalVoteSlot) error {
	if block.Hash() != confirm.Hash {
		return errors.New("block confirmation validate failed")
	}

	// TODO need to implement
	fmt.Println("CheckBlockWithConfirmation Tracer ...")

	return nil
}
