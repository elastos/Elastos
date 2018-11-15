package blockchain

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"
)

func CheckConfirm(confirm *core.DPosProposalVoteSlot) error {
	signers := make(map[string]struct{})
	sponsors := make(map[string]struct{})
	for _, vote := range confirm.Votes {
		if !vote.IsValid() {
			return errors.New("[onConfirm] confirm contain invalid vote")
		}
		if !vote.Proposal.IsValid() {
			return errors.New("[onConfirm] confirm contain invalid proposal")
		}
		signers[vote.Signer] = struct{}{}
		sponsors[vote.Proposal.Sponsor] = struct{}{}
	}

	if len(signers) < int(config.Parameters.ArbiterConfiguration.MajorityCount) {
		return errors.New("[onConfirm] signers less than majority count")
	}

	if len(sponsors) != 1 {
		return errors.New("[onConfirm] different sponsors in votes")
	}

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
