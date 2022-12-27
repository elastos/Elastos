// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"errors"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/checkpoint"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
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
		if !vote.Accept {
			continue
		}
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

func checkBlockWithConfirmation(block *Block, confirm *payload.Confirm,
	manager *checkpoint.Manager) error {
	if block.Hash() != confirm.Proposal.BlockHash {
		return errors.New("[CheckBlockWithConfirmation] block " +
			"confirmation validate failed")
	}

	if err := ConfirmContextCheck(confirm); err != nil {
		// rollback to the state before this method
		if e := manager.OnRollbackTo(block.Height - 1); e != nil {
			panic("rollback fail when check block with confirmation")
		}
		return err
	}

	return nil
}

func PreProcessSpecialTx(block *Block) error {
	illegalBlocks := make([]*payload.DPOSIllegalBlocks, 0)
	inactivePayloads := make([]*payload.InactiveArbitrators, 0)
	for _, tx := range block.Transactions {
		switch tx.TxType {
		case InactiveArbitrators:
			if err := CheckInactiveArbitrators(tx); err != nil {
				return err
			}
			if err := checkTransactionSignature(tx, map[*Input]Output{}); err != nil {
				return err
			}

			inactivePayloads = append(inactivePayloads,
				tx.Payload.(*payload.InactiveArbitrators))
		case IllegalBlockEvidence:
			p, ok := tx.Payload.(*payload.DPOSIllegalBlocks)
			if !ok {
				return errors.New("invalid payload")
			}
			if err := CheckDPOSIllegalBlocks(p); err != nil {
				return err
			}

			illegalBlocks = append(illegalBlocks, p)
		}
	}

	if len(illegalBlocks) != 0 {
		for _, v := range illegalBlocks {
			if err := DefaultLedger.Arbitrators.ProcessSpecialTxPayload(
				v, block.Height-1); err != nil {
				return errors.New("force change fail when finding an " +
					"inactive arbitrators transaction")
			}
		}
	}
	if len(inactivePayloads) != 0 {
		for _, v := range inactivePayloads {
			if err := DefaultLedger.Arbitrators.ProcessSpecialTxPayload(
				v, block.Height-1); err != nil {
				return errors.New("force change fail when finding an " +
					"inactive arbitrators transaction")
			}
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

func ProposalCheckByHeight(proposal *payload.DPOSProposal,
	height uint32) error {
	if err := ProposalSanityCheck(proposal); err != nil {
		return err
	}

	if err := ProposalContextCheckByHeight(proposal, height); err != nil {
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
	arbiters := DefaultLedger.Arbitrators.GetArbitrators()
	var isArbiter bool
	for _, a := range arbiters {
		if !a.IsNormal {
			continue
		}
		if bytes.Equal(a.NodePublicKey, proposal.Sponsor) {
			isArbiter = true
		}
	}
	if !isArbiter {
		return errors.New("current arbitrators verify error")
	}

	return nil
}

func ProposalContextCheckByHeight(proposal *payload.DPOSProposal,
	height uint32) error {
	var isArbiter bool
	keyFrames := DefaultLedger.Arbitrators.GetSnapshot(height)
out:
	for _, k := range keyFrames {
		for _, a := range k.CurrentArbitrators {
			if bytes.Equal(a.GetNodePublicKey(), proposal.Sponsor) {
				isArbiter = true
				break out
			}
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

func VoteCheckByHeight(vote *payload.DPOSProposalVote, height uint32) error {
	if err := VoteSanityCheck(vote); err != nil {
		return err
	}

	if err := VoteContextCheckByHeight(vote, height); err != nil {
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
	arbiters := DefaultLedger.Arbitrators.GetArbitrators()
	var isArbiter bool
	for _, a := range arbiters {
		if !a.IsNormal {
			continue
		}
		if bytes.Equal(a.NodePublicKey, vote.Signer) {
			isArbiter = true
		}
	}
	if !isArbiter {
		return errors.New("current arbitrators verify error")
	}

	return nil
}

func VoteContextCheckByHeight(vote *payload.DPOSProposalVote,
	height uint32) error {
	var isArbiter bool
	keyFrames := DefaultLedger.Arbitrators.GetSnapshot(height)
out:
	for _, k := range keyFrames {
		for _, a := range k.CurrentArbitrators {
			if bytes.Equal(a.GetNodePublicKey(), vote.Signer) {
				isArbiter = true
				break out
			}
		}
	}
	if !isArbiter {
		return errors.New("current arbitrators verify error")
	}

	return nil
}
