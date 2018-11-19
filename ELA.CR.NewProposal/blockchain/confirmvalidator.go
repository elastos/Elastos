package blockchain

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func CheckConfirm(confirm *core.DPosProposalVoteSlot) error {
	signers := make(map[string]struct{})
	sponsors := make(map[string]struct{})
	for _, vote := range confirm.Votes {
		if !IsVoteValid(&vote) {
			return errors.New("[onConfirm] confirm contain invalid vote")
		}
		if !IsProposalValid(&vote.Proposal) {
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

func IsProposalValid(proposal *core.DPosProposal) bool {
	var isArbiter bool
	for _, a := range config.Parameters.Arbiters {
		if a == proposal.Sponsor {
			isArbiter = true
		}
	}
	if !isArbiter {
		return false
	}

	publicKey, err := common.HexStringToBytes(proposal.Sponsor)
	if err != nil {
		return false
	}
	pubKey, err := crypto.DecodePoint(publicKey)
	if err != nil {
		return false
	}
	err = crypto.Verify(*pubKey, proposal.Data(), proposal.Sign)
	if err != nil {
		return false
	}

	return true
}

func IsVoteValid(vote *core.DPosProposalVote) bool {
	var isArbiter bool
	for _, a := range config.Parameters.Arbiters {
		if a == vote.Signer {
			isArbiter = true
		}
	}
	if !isArbiter {
		return false
	}

	publicKey, err := common.HexStringToBytes(vote.Signer)
	if err != nil {
		return false
	}
	pubKey, err := crypto.DecodePoint(publicKey)
	if err != nil {
		return false
	}
	err = crypto.Verify(*pubKey, vote.Data(), vote.Sign)
	if err != nil {
		return false
	}

	return true
}
