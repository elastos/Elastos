package blockchain

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

func ConfirmSanityCheck(confirm *payload.Confirm) error {
	if err := ProposalSanityCheck(&confirm.Proposal); err != nil {
		return errors.New("[ConfirmSanityCheck] confirm contain invalid " +
			"proposal: " + err.Error())
	}

	proposalHash := confirm.Proposal.Hash()
	for _, vote := range confirm.Votes {
		if !vote.Accept {
			return errors.New("[ConfirmSanityCheck] confirm contains " +
				"reject vote")
		}

		if !proposalHash.IsEqual(vote.ProposalHash) {
			return errors.New("[ConfirmSanityCheck] confirm contains " +
				"invalid vote")
		}

		if err := VoteSanityCheck(&vote); err != nil {
			return errors.New("[ConfirmSanityCheck] confirm contain invalid " +
				"vote: " + err.Error())
		}
	}

	return nil
}

func ConfirmContextCheck(confirm *payload.Confirm) error {
	signers := make(map[string]struct{})
	for _, vote := range confirm.Votes {
		signers[common.BytesToHexString(vote.Signer)] = struct{}{}
	}

	if len(signers) <= DefaultLedger.Arbitrators.GetArbitersMajorityCount() {
		return errors.New("[ConfirmContextCheck] signers less than " +
			"majority count")
	}

	if err := ProposalContextCheck(&confirm.Proposal); err != nil {
		return errors.New("[ConfirmContextCheck] confirm contain invalid " +
			"proposal: " + err.Error())
	}

	for _, vote := range confirm.Votes {
		if err := VoteContextCheck(&vote); err != nil {
			return errors.New("[ConfirmContextCheck] confirm contain invalid " +
				"vote: " + err.Error())
		}
	}

	return nil
}

func checkBlockWithConfirmation(block *Block, confirm *payload.Confirm) error {
	if block.Hash() != confirm.Proposal.BlockHash {
		return errors.New("[CheckBlockWithConfirmation] block " +
			"confirmation validate failed")
	}

	var inactivePayload *payload.InactiveArbitrators
	for _, tx := range block.Transactions {
		if tx.IsInactiveArbitrators() {
			inactivePayload = tx.Payload.(*payload.InactiveArbitrators)
			break
		}
	}

	if err := ConfirmContextCheck(confirm); err != nil {
		// rollback to the state before this method
		if e := DefaultLedger.Blockchain.state.RollbackTo(block.
			Height - 1); e != nil {
			panic("rollback fail when check block with confirmation")
		}
		return err
	} else if inactivePayload != nil {
		if err := DefaultLedger.Arbitrators.ProcessSpecialTxPayload(
			inactivePayload, block.Height); err != nil {
			panic("force change fail when finding an inactive arbitrators" +
				" transaction")
		}
	}

	return nil
}

func ProposalCheck(proposal *payload.DPOSProposal) error {
	if err := ProposalSanityCheck(proposal); err != nil {
		return err
	}

	if err := ProposalContextCheck(proposal); err != nil {
		return err
	}

	return nil
}

func ProposalSanityCheck(proposal *payload.DPOSProposal) error {
	pubKey, err := crypto.DecodePoint(proposal.Sponsor)
	if err != nil {
		return err
	}
	err = crypto.Verify(*pubKey, proposal.Data(), proposal.Sign)
	if err != nil {
		return err
	}

	return nil
}

func ProposalContextCheck(proposal *payload.DPOSProposal) error {
	var isArbiter bool
	arbiters := DefaultLedger.Arbitrators.GetArbitrators()
	for _, a := range arbiters {
		if bytes.Equal(a, proposal.Sponsor) {
			isArbiter = true
		}
	}
	if !isArbiter {
		return errors.New("current arbitrators verify error")
	}

	return nil
}

func VoteCheck(vote *payload.DPOSProposalVote) error {
	if err := VoteSanityCheck(vote); err != nil {
		return err
	}

	if err := VoteContextCheck(vote); err != nil {
		log.Warn("[VoteContextCheck] error: ", err.Error())
		return err
	}

	return nil
}

func VoteSanityCheck(vote *payload.DPOSProposalVote) error {
	pubKey, err := crypto.DecodePoint(vote.Signer)
	if err != nil {
		return err
	}
	err = crypto.Verify(*pubKey, vote.Data(), vote.Sign)
	if err != nil {
		return err
	}

	return nil
}
func VoteContextCheck(vote *payload.DPOSProposalVote) error {
	var isArbiter bool
	arbiters := DefaultLedger.Arbitrators.GetArbitrators()
	for _, a := range arbiters {
		if bytes.Equal(a, vote.Signer) {
			isArbiter = true
		}
	}
	if !isArbiter {
		return errors.New("current arbitrators verify error")
	}

	return nil
}
