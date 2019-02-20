package blockchain

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/crypto"
)

func CheckConfirm(confirm *DPosProposalVoteSlot) error {
	if !IsProposalValid(&confirm.Proposal) {
		return errors.New("[onConfirm] confirm contain invalid proposal")
	}

	signers := make(map[string]struct{})
	proposalHash := confirm.Proposal.Hash()
	for _, vote := range confirm.Votes {
		if !proposalHash.IsEqual(vote.ProposalHash) || !IsVoteValid(&vote) {
			return errors.New("[onConfirm] confirm contain invalid vote")
		}

		signers[vote.Signer] = struct{}{}
	}

	if len(signers) < int(config.MajorityCount) {
		return errors.New("[onConfirm] signers less than majority count")
	}

	return nil
}

func CheckBlockWithConfirmation(block *Block, confirm *DPosProposalVoteSlot) error {
	if block.Hash() != confirm.Hash {
		return errors.New("block confirmation validate failed")
	}

	// TODO need to implement
	fmt.Println("CheckBlockWithConfirmation Tracer ...")

	return nil
}

func IsProposalValid(proposal *DPosProposal) bool {
	var isArbiter bool
	for _, a := range DefaultLedger.Arbitrators.GetArbitrators() {
		pubStr := common.BytesToHexString(a)
		if pubStr == proposal.Sponsor {
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

func IsVoteValid(vote *DPosProposalVote) bool {
	var isArbiter bool
	for _, a := range DefaultLedger.Arbitrators.GetArbitrators() {
		pubStr := common.BytesToHexString(a)
		if pubStr == vote.Signer {
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
