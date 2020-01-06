// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// ProcessBlock takes a block and it's confirm to update CR state and
// votes accordingly.
func (c *Committee) ProcessBlockInVotingPeriod(block *types.Block) {
	c.processTransactions(block.Transactions, block.Height)
	c.state.history.Commit(block.Height)
}

// processTransactions takes the transactions and the height when they have been
// packed into a block.  Then loop through the transactions to update CR
// state and votes according to transactions content.
func (c *Committee) processTransactions(txs []*types.Transaction, height uint32) {
	for _, tx := range txs {
		c.processTransaction(tx, height)
	}

	// Check if any pending candidates has got 6 confirms, set them to activate.
	activateCandidateFromPending :=
		func(key common.Uint168, candidate *Candidate) {
			c.state.history.Append(height, func() {
				candidate.state = Active
				c.state.Candidates[key] = candidate
			}, func() {
				candidate.state = Pending
				c.state.Candidates[key] = candidate
			})
		}

	pendingCandidates := c.state.getCandidates(Pending)

	if len(pendingCandidates) > 0 {
		for _, candidate := range pendingCandidates {
			if height-candidate.registerHeight+1 >= ActivateDuration {
				activateCandidateFromPending(candidate.info.DID, candidate)
			}
		}
	}
}

// processTransaction take a transaction and the height it has been packed into
// a block, then update producers state and votes according to the transaction
// content.
func (c *Committee) processTransaction(tx *types.Transaction, height uint32) {
	switch tx.TxType {
	case types.RegisterCR:
		c.state.registerCR(tx, height)

	case types.UpdateCR:
		c.state.updateCR(tx.Payload.(*payload.CRInfo), height)

	case types.UnregisterCR:
		c.state.unregisterCR(tx.Payload.(*payload.UnregisterCR), height)

	case types.TransferAsset:
		c.processVotes(tx, height)
		c.state.processDeposit(tx, height)

	case types.ReturnCRDepositCoin:
		c.state.returnDeposit(tx, height)
		c.state.processDeposit(tx, height)

	case types.CRCProposal:
		c.manager.registerProposal(tx, height, c.state.history)

	case types.CRCProposalReview:
		c.manager.proposalReview(tx, height, c.state.history)

	case types.CRCProposalTracking:
		c.manager.proposalTracking(tx, height, c.state.history)

	case types.CRCProposalWithdraw:
		c.manager.proposalWithdraw(tx, height, c.state.history)

	case types.CRCAppropriation:
		c.processCRCAppropriation(tx, height, c.state.history)
	}

	c.state.processCancelVotes(tx, height)
	c.processCRCAddressRelatedTx(tx, height)
}

// processBlockInElectionPeriod takes a block and it's confirm to update CR member state
// and proposals accordingly, only in election period and not in voting period.
func (c *Committee) processBlockInElectionPeriod(block *types.Block) {
	for _, tx := range block.Transactions {
		c.processElectionTransaction(tx, block.Height)
	}
	c.state.history.Commit(block.Height)
}

// processElectionTransaction take a transaction and the height it has been
// packed into a block, then update CR members state and proposals according to
// the transaction content.
func (c *Committee) processElectionTransaction(tx *types.Transaction, height uint32) {
	switch tx.TxType {
	case types.TransferAsset:
		c.processVotes(tx, height)
		c.state.processDeposit(tx, height)

	case types.ReturnCRDepositCoin:
		c.state.returnDeposit(tx, height)
		c.state.processDeposit(tx, height)

	case types.CRCProposal:
		c.manager.registerProposal(tx, height, c.state.history)

	case types.CRCProposalReview:
		c.manager.proposalReview(tx, height, c.state.history)

	case types.CRCAppropriation:
		c.processCRCAppropriation(tx, height, c.state.history)

	case types.CRCProposalTracking:
		c.manager.proposalTracking(tx, height, c.state.history)

	case types.CRCProposalWithdraw:
		c.manager.proposalWithdraw(tx, height, c.state.history)
	}

	c.state.processCancelVotes(tx, height)
	c.processCRCAddressRelatedTx(tx, height)

}

// processVotes takes a transaction, if the transaction including any vote
// inputs or outputs, validate and update CR votes.
func (c *Committee) processVotes(tx *types.Transaction, height uint32) {
	if tx.Version >= types.TxVersion09 {
		for i, output := range tx.Outputs {
			if output.Type != types.OTVote {
				continue
			}
			p, _ := output.Payload.(*outputpayload.VoteOutput)
			if p.Version < outputpayload.VoteProducerAndCRVersion {
				continue
			}

			// process CRC content
			var exist bool
			for _, content := range p.Contents {
				if content.VoteType == outputpayload.CRC ||
					content.VoteType == outputpayload.CRCProposal ||
					content.VoteType == outputpayload.CRCImpeachment {
					exist = true
					break
				}
			}
			if exist {
				op := types.NewOutPoint(tx.Hash(), uint16(i))
				c.state.history.Append(height, func() {
					c.state.Votes[op.ReferKey()] = struct{}{}
				}, func() {
					delete(c.state.Votes, op.ReferKey())
				})
				c.processVoteOutput(output, height)
			}
		}
	}
}

// processVoteOutput takes a transaction output with vote payload.
func (c *Committee) processVoteOutput(output *types.Output, height uint32) {
	p := output.Payload.(*outputpayload.VoteOutput)
	for _, vote := range p.Contents {
		for _, cv := range vote.CandidateVotes {
			switch vote.VoteType {
			case outputpayload.CRC:
				c.state.processVoteCRC(height, cv)

			case outputpayload.CRCProposal:
				c.state.processVoteCRCProposal(height, cv)

			case outputpayload.CRCImpeachment:
				c.processImpeachment(height, cv.Candidate, cv.Votes, c.state.history)
			}
		}
	}
}

// processCRCRelatedAmount takes a transaction, if the transaction takes a previous
// output to CRC related address then try to subtract the vote.
func (c *Committee) processCRCAddressRelatedTx(tx *types.Transaction, height uint32) {
	for i, output := range tx.Outputs {
		if output.ProgramHash.IsEqual(c.params.CRCFoundation) {
			op := types.NewOutPoint(tx.Hash(), uint16(i))
			c.state.CRCFoundationOutputs[op.ReferKey()] = output.Value
		} else if output.ProgramHash.IsEqual(c.params.CRCCommitteeAddress) {
			op := types.NewOutPoint(tx.Hash(), uint16(i))
			c.state.CRCCommitteeOutputs[op.ReferKey()] = output.Value
		}
	}

	c.processCRCRelatedAmount(tx, height, c.state.history,
		c.state.CRCFoundationOutputs, c.state.CRCCommitteeOutputs)
}
