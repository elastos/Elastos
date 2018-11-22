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

	if len(signers) < int(config.Parameters.ArbiterConfiguration.MajorityCount) {
		return errors.New("[onConfirm] signers less than majority count")
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

func IsIllegalProposalsValid(d *core.DposIllegalProposals) bool {

	// proposal hash and block should match
	if !d.Evidence.IsMatch() || !d.CompareEvidence.IsMatch() {
		return false
	}

	// should be in same height
	if d.Evidence.BlockHeader.Height != d.CompareEvidence.BlockHeader.Height {
		return false
	}

	// proposals can not be same
	if d.Evidence.Proposal.Hash().IsEqual(d.CompareEvidence.Proposal.Hash()) {
		return false
	}

	// should be same sponsor
	if d.Evidence.Proposal.Sponsor != d.CompareEvidence.Proposal.Sponsor {
		return false
	}

	// should in same view
	if d.Evidence.Proposal.ViewOffset != d.Evidence.Proposal.ViewOffset {
		return false
	}

	// proposal should be valid
	if !IsProposalValid(&d.Evidence.Proposal) || !IsProposalValid(&d.Evidence.Proposal) {
		return false
	}

	return true
}

func IsIllegalVotesValid(d *core.DposIllegalVotes) bool {

	// vote, proposal and block should match
	if !d.Evidence.IsMatch() || !d.CompareEvidence.IsMatch() {
		return false
	}

	// should be in same height
	if d.Evidence.BlockHeader.Height != d.CompareEvidence.BlockHeader.Height {
		return false
	}

	// votes can not be same
	if d.Evidence.Vote.Hash().IsEqual(d.CompareEvidence.Vote.Hash()) {
		return false
	}

	// should be same signer
	if d.Evidence.Vote.Signer != d.CompareEvidence.Vote.Signer {
		return false
	}

	// should in same view
	if d.Evidence.Proposal.ViewOffset != d.CompareEvidence.Proposal.ViewOffset {
		return false
	}

	if !IsProposalValid(&d.Evidence.Proposal) || IsProposalValid(&d.CompareEvidence.Proposal) ||
		!IsVoteValid(&d.Evidence.Vote) || IsVoteValid(&d.CompareEvidence.Vote) {
		return false
	}

	return true
}
