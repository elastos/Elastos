package blockchain

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

func CheckConfirm(confirm *msg.DPosProposalVoteSlot) error {
	// TODO need to implement
	fmt.Println("CheckConfirm Tracer ...")
	return nil
}

func CheckBlockWithConfirmation(block *core.Block, confirm *msg.DPosProposalVoteSlot) error {
	if block.Hash() != confirm.Hash {
		return errors.New("block confirmation validate failed")
	}

	// TODO need to implement
	fmt.Println("CheckBlockWithConfirmation Tracer ...")

	return nil
}
