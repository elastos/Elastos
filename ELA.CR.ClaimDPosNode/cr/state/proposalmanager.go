// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/utils"
)

type ProposalStatus uint8

const (
	// Registered is the status means the CRC proposal tx has been on the best
	//	chain.
	Registered ProposalStatus = 0x00

	// CRAgreed means CRC has agreed the proposal.
	CRAgreed ProposalStatus = 0x01

	// VoterAgreed means there are not enough negative vote about the proposal.
	VoterAgreed ProposalStatus = 0x02

	// Finished means the proposal has run out the lifetime.
	Finished ProposalStatus = 0x03

	// CRCanceled means the proposal canceled by CRC voting.
	CRCanceled ProposalStatus = 0x04

	// VoterCanceled means the proposal canceled by voters' reject voting.
	VoterCanceled ProposalStatus = 0x05

	// Terminated means proposal had been approved by both CR and voters,
	// whoever the proposal related project has been decided to terminate for
	// some reason.
	Terminated ProposalStatus = 0x06

	// Aborted means the proposal was cancelled because of a snap election.
	Aborted ProposalStatus = 0x07
)

func (status ProposalStatus) String() string {
	switch status {
	case Registered:
		return "Registered"
	case CRAgreed:
		return "CRAgreed"
	case VoterAgreed:
		return "VoterAgreed"
	case Finished:
		return "Finished"
	case CRCanceled:
		return "CRCanceled"
	case VoterCanceled:
		return "VoterCanceled"
	case Terminated:
		return "Terminated"
	case Aborted:
		return "Aborted"
	default:
		return fmt.Sprintf("Unknown ProposalStatus (%d)", status)
	}
}

// ProposalManager used to manage all proposals existing in block chain.
type ProposalManager struct {
	ProposalKeyFrame
	params  *config.Params
	history *utils.History
}

// existDraft judge if specified draft (that related to a proposal) exist.
func (p *ProposalManager) existDraft(hash common.Uint256) bool {
	for _, v := range p.Proposals {
		if v.Proposal.DraftHash.IsEqual(hash) {
			return true
		}
	}
	return false
}

// existProposal judge if specified proposal exist.
func (p *ProposalManager) existProposal(hash common.Uint256) bool {
	_, ok := p.Proposals[hash]
	return ok
}

func (p *ProposalManager) getAllProposals() (dst ProposalsMap) {
	dst = NewProposalMap()
	for k, v := range p.Proposals {
		p := *v
		dst[k] = &p
	}
	return
}

func (p *ProposalManager) getProposalByDraftHash(draftHash common.Uint256) *ProposalState {
	for _, v := range p.Proposals {
		if v.Proposal.DraftHash.IsEqual(draftHash) {
			return v
		}
	}
	return nil
}

func (p *ProposalManager) getProposals(status ProposalStatus) (dst ProposalsMap) {
	dst = NewProposalMap()
	for k, v := range p.Proposals {
		if v.Status == status {
			p := *v
			dst[k] = &p
		}
	}
	return
}

// getProposal will return a proposal with specified hash,
// and return nil if not found.
func (p *ProposalManager) getProposal(hash common.Uint256) *ProposalState {
	result, ok := p.Proposals[hash]
	if !ok {
		return nil
	}
	return result
}

func (p *ProposalManager) availableWithdrawalAmount(hash common.Uint256) common.Fixed64 {
	proposal := p.getProposal(hash)
	amount := common.Fixed64(0)
	if proposal == nil {
		return amount
	}
	for i, a := range proposal.WithdrawableBudgets {
		if _, ok := proposal.WithdrawnBudgets[i]; !ok {
			amount += a
		}
	}
	return amount
}

func getProposalTotalBudgetAmount(proposal payload.CRCProposal) common.Fixed64 {
	var budget common.Fixed64
	for _, b := range proposal.Budgets {
		budget += b.Amount
	}
	return budget
}

func getProposalUnusedBudgetAmount(proposalState *ProposalState) common.Fixed64 {
	var budget common.Fixed64
	for _, b := range proposalState.Proposal.Budgets {
		if _, ok := proposalState.WithdrawnBudgets[b.Stage]; !ok {
			budget += b.Amount
		}
	}
	return budget
}

// updateProposals will update proposals' status.
func (p *ProposalManager) updateProposals(height uint32,
	circulation common.Fixed64, inElectionPeriod bool) common.Fixed64 {
	var unusedAmount common.Fixed64
	for _, v := range p.Proposals {
		switch v.Status {
		case Registered:
			if !inElectionPeriod {
				p.abortProposal(v, height)
				unusedAmount += getProposalTotalBudgetAmount(v.Proposal)
				break
			}
			if p.shouldEndCRCVote(v.RegisterHeight, height) {
				if p.transferRegisteredState(v, height) == CRCanceled {
					unusedAmount += getProposalTotalBudgetAmount(v.Proposal)
				}
			}
		case CRAgreed:
			if !inElectionPeriod {
				p.abortProposal(v, height)
				unusedAmount += getProposalTotalBudgetAmount(v.Proposal)
				break
			}
			if p.shouldEndPublicVote(v.VoteStartHeight, height) {
				if p.transferCRAgreedState(v, height, circulation) == VoterCanceled {
					unusedAmount += getProposalTotalBudgetAmount(v.Proposal)
					continue
				}
				if v.Proposal.ProposalType == payload.CloseProposal {
					closeProposal := p.Proposals[v.Proposal.CloseProposalHash]
					unusedAmount += getProposalUnusedBudgetAmount(closeProposal)
					p.terminatedProposal(closeProposal, height)
				}
			}
		}
	}

	return unusedAmount
}

// abortProposal will transfer the status to aborted.
func (p *ProposalManager) abortProposal(proposalState *ProposalState,
	height uint32) {
	oriStatus := proposalState.Status
	oriBudgetsStatus := make(map[uint8]BudgetStatus)
	for k, v := range proposalState.BudgetsStatus {
		oriBudgetsStatus[k] = v
	}
	p.history.Append(height, func() {
		proposalState.Status = Aborted
		for k, _ := range proposalState.BudgetsStatus {
			proposalState.BudgetsStatus[k] = Closed
		}
	}, func() {
		proposalState.Status = oriStatus
		proposalState.BudgetsStatus = oriBudgetsStatus
	})
}

// abortProposal will transfer the status to aborted.
func (p *ProposalManager) terminatedProposal(proposalState *ProposalState,
	height uint32) {
	oriStatus := proposalState.Status
	oriBudgetsStatus := make(map[uint8]BudgetStatus)
	for k, v := range proposalState.BudgetsStatus {
		oriBudgetsStatus[k] = v
	}
	p.history.Append(height, func() {
		proposalState.Status = Terminated
		for k, v := range proposalState.BudgetsStatus {
			if v == Unfinished || v == Rejected {
				proposalState.BudgetsStatus[k] = Closed
			}
		}
	}, func() {
		proposalState.Status = oriStatus
		proposalState.BudgetsStatus = oriBudgetsStatus
	})
}

// transferRegisteredState will transfer the Registered state by CR agreement
// count.
func (p *ProposalManager) transferRegisteredState(proposalState *ProposalState,
	height uint32) (status ProposalStatus) {
	agreedCount := uint32(0)
	for _, v := range proposalState.CRVotes {
		if v == payload.Approve {
			agreedCount++
		}
	}

	oriVoteStartHeight := proposalState.VoteStartHeight
	if agreedCount >= p.params.CRAgreementCount {
		status = CRAgreed
		p.history.Append(height, func() {
			proposalState.Status = CRAgreed
			proposalState.VoteStartHeight = height
		}, func() {
			proposalState.Status = Registered
			proposalState.VoteStartHeight = oriVoteStartHeight
		})
	} else {
		status = CRCanceled
		oriBudgetsStatus := make(map[uint8]BudgetStatus)
		for k, v := range proposalState.BudgetsStatus {
			oriBudgetsStatus[k] = v
		}
		p.history.Append(height, func() {
			proposalState.Status = CRCanceled
			for k, _ := range proposalState.BudgetsStatus {
				proposalState.BudgetsStatus[k] = Closed
			}
		}, func() {
			proposalState.Status = Registered
			proposalState.BudgetsStatus = oriBudgetsStatus
		})
	}
	return
}

// transferCRAgreedState will transfer CRAgreed state by votes' reject amount.
func (p *ProposalManager) transferCRAgreedState(proposalState *ProposalState,
	height uint32, circulation common.Fixed64) (status ProposalStatus) {
	if proposalState.VotersRejectAmount >= common.Fixed64(float64(circulation)*
		p.params.VoterRejectPercentage/100.0) {
		status = VoterCanceled
		oriBudgetsStatus := make(map[uint8]BudgetStatus)
		for k, v := range proposalState.BudgetsStatus {
			oriBudgetsStatus[k] = v
		}
		p.history.Append(height, func() {
			proposalState.Status = VoterCanceled
			for k, _ := range proposalState.BudgetsStatus {
				proposalState.BudgetsStatus[k] = Closed
			}
		}, func() {
			proposalState.Status = CRAgreed
			proposalState.BudgetsStatus = oriBudgetsStatus
		})
	} else {
		status = VoterAgreed
		p.history.Append(height, func() {
			proposalState.Status = VoterAgreed
			for _, b := range proposalState.Proposal.Budgets {
				if b.Type == payload.Imprest {
					proposalState.WithdrawableBudgets[b.Stage] = b.Amount
					break
				}
			}
		}, func() {
			proposalState.Status = CRAgreed
			for _, b := range proposalState.Proposal.Budgets {
				if b.Type == payload.Imprest {
					delete(proposalState.WithdrawableBudgets, b.Stage)
					break
				}
			}

		})
	}
	return
}

// shouldEndCRCVote returns if current height should end CRC vote about
// 	the specified proposal.
func (p *ProposalManager) shouldEndCRCVote(RegisterHeight uint32,
	height uint32) bool {
	//proposal.RegisterHeight
	return RegisterHeight+p.params.ProposalCRVotingPeriod <= height
}

// shouldEndPublicVote returns if current height should end public vote
// about the specified proposal.
func (p *ProposalManager) shouldEndPublicVote(VoteStartHeight uint32,
	height uint32) bool {
	return VoteStartHeight+p.params.ProposalPublicVotingPeriod <=
		height
}

func (p *ProposalManager) isProposalFull(did common.Uint168) bool {
	return p.getProposalCount(did) >= int(p.params.MaxCommitteeProposalCount)
}

func (p *ProposalManager) getProposalCount(did common.Uint168) int {
	proposalHashsSet, ok := p.ProposalHashes[did]
	if !ok {
		return 0
	}
	return proposalHashsSet.Len()
}

func (p *ProposalManager) addProposal(did common.Uint168,
	proposalHash common.Uint256) {
	proposalHashesSet, ok := p.ProposalHashes[did]
	if !ok {
		proposalHashesSet = NewProposalHashSet()
		proposalHashesSet.Add(proposalHash)
		p.ProposalHashes[did] = proposalHashesSet
		return
	}
	proposalHashesSet.Add(proposalHash)
}

func (p *ProposalManager) delProposal(did common.Uint168,
	proposalHash common.Uint256) {
	proposalHashesSet, ok := p.ProposalHashes[did]
	if ok {
		if len(proposalHashesSet) == 1 {
			delete(p.ProposalHashes, did)
			return
		}
		proposalHashesSet.Remove(proposalHash)
	}
}

// registerProposal will register proposal state in proposal manager
func (p *ProposalManager) registerProposal(tx *types.Transaction,
	height uint32, currentsSession uint64, history *utils.History) {
	proposal := tx.Payload.(*payload.CRCProposal)
	//The number of the proposals of the committee can not more than 128
	if p.isProposalFull(proposal.CRCouncilMemberDID) {
		return
	}
	budgetsStatus := make(map[uint8]BudgetStatus)
	for _, budget := range proposal.Budgets {
		if budget.Type == payload.Imprest {
			budgetsStatus[budget.Stage] = Withdrawable
			continue
		}
		budgetsStatus[budget.Stage] = Unfinished
	}
	proposalState := &ProposalState{
		Status:              Registered,
		Proposal:            *proposal,
		TxHash:              tx.Hash(),
		CRVotes:             map[common.Uint168]payload.VoteResult{},
		VotersRejectAmount:  common.Fixed64(0),
		RegisterHeight:      height,
		VoteStartHeight:     0,
		WithdrawnBudgets:    make(map[uint8]common.Fixed64),
		WithdrawableBudgets: make(map[uint8]common.Fixed64),
		BudgetsStatus:       budgetsStatus,
		FinalPaymentStatus:  false,
		TrackingCount:       0,
		TerminatedHeight:    0,
		ProposalOwner:       proposal.OwnerPublicKey,
	}
	crCouncilMemberDID := proposal.CRCouncilMemberDID
	hash := proposal.Hash()

	history.Append(height, func() {
		p.Proposals[proposal.Hash()] = proposalState
		p.addProposal(crCouncilMemberDID, hash)
		if _, ok := p.ProposalSession[currentsSession]; !ok {
			p.ProposalSession[currentsSession] = make([]common.Uint256, 0)
		}
		p.ProposalSession[currentsSession] =
			append(p.ProposalSession[currentsSession], proposal.Hash())
	}, func() {
		delete(p.Proposals, proposal.Hash())
		p.delProposal(crCouncilMemberDID, hash)
		if len(p.ProposalSession[currentsSession]) == 1 {
			delete(p.ProposalSession, currentsSession)
		} else {
			count := len(p.ProposalSession[currentsSession])
			p.ProposalSession[currentsSession] = p.ProposalSession[currentsSession][:count-1]
		}
	})
}

func getCIDByCode(code []byte) (*common.Uint168, error) {
	ct1, err := contract.CreateCRIDContractByCode(code)
	if err != nil {
		return nil, err
	}
	return ct1.ToProgramHash(), err
}

func getDIDByCode(code []byte) (*common.Uint168, error) {
	didCode := make([]byte, len(code))
	copy(didCode, code)
	didCode = append(didCode[:len(code)-1], common.DID)
	ct1, err := contract.CreateCRIDContractByCode(didCode)
	if err != nil {
		return nil, err
	}
	return ct1.ToProgramHash(), err
}

func getCIDByPublicKey(publicKey []byte) (*common.Uint168, error) {
	pubkey, err := crypto.DecodePoint(publicKey)
	if err != nil {
		return nil, err
	}
	code, err := contract.CreateStandardRedeemScript(pubkey)
	if err != nil {
		return nil, err
	}
	ct, err := contract.CreateCRIDContractByCode(code)
	if err != nil {
		return nil, err
	}
	return ct.ToProgramHash(), nil
}

func (p *ProposalManager) proposalReview(tx *types.Transaction,
	height uint32, history *utils.History) {
	proposalReview := tx.Payload.(*payload.CRCProposalReview)
	proposalState := p.getProposal(proposalReview.ProposalHash)
	if proposalState == nil {
		return
	}
	did := proposalReview.DID
	oldVoteResult, oldVoteExist := proposalState.CRVotes[did]
	history.Append(height, func() {
		proposalState.CRVotes[did] = proposalReview.VoteResult
	}, func() {
		if oldVoteExist {
			proposalState.CRVotes[did] = oldVoteResult
		} else {
			delete(proposalState.CRVotes, did)
		}

	})
}

func (p *ProposalManager) proposalWithdraw(tx *types.Transaction,
	height uint32, history *utils.History) {
	withdrawPayload := tx.Payload.(*payload.CRCProposalWithdraw)
	proposalState := p.getProposal(withdrawPayload.ProposalHash)
	if proposalState == nil {
		return
	}
	withdrawingBudgets := make(map[uint8]common.Fixed64)
	for i, a := range proposalState.WithdrawableBudgets {
		if _, ok := proposalState.WithdrawnBudgets[i]; !ok {
			withdrawingBudgets[i] = a
		}
	}
	oriBudgetsStatus := make(map[uint8]BudgetStatus)
	for k, v := range proposalState.BudgetsStatus {
		oriBudgetsStatus[k] = v
	}
	history.Append(height, func() {
		for k, v := range withdrawingBudgets {
			proposalState.WithdrawnBudgets[k] = v
		}
		for k, v := range proposalState.BudgetsStatus {
			if v == Withdrawable {
				proposalState.BudgetsStatus[k] = Withdrawn

			}
		}
	}, func() {
		for k, _ := range withdrawingBudgets {
			delete(proposalState.WithdrawnBudgets, k)
		}
		proposalState.BudgetsStatus = oriBudgetsStatus
	})
}

func (p *ProposalManager) proposalTracking(tx *types.Transaction,
	height uint32, history *utils.History) (unusedBudget common.Fixed64) {
	proposalTracking := tx.Payload.(*payload.CRCProposalTracking)
	proposalState := p.getProposal(proposalTracking.ProposalHash)
	if proposalState == nil {
		return
	}

	trackingType := proposalTracking.ProposalTrackingType
	owner := proposalState.ProposalOwner
	terminatedHeight := proposalState.TerminatedHeight
	status := proposalState.Status
	oriBudgetsStatus := make(map[uint8]BudgetStatus)
	for k, v := range proposalState.BudgetsStatus {
		oriBudgetsStatus[k] = v
	}

	if trackingType == payload.Terminated {
		for _, budget := range proposalState.Proposal.Budgets {
			if _, ok := proposalState.WithdrawableBudgets[budget.Stage]; !ok {
				unusedBudget += budget.Amount
			}
		}
	}
	if trackingType == payload.Finalized {
		for _, budget := range proposalState.Proposal.Budgets {
			if budget.Type == payload.FinalPayment {
				continue
			}
			if _, ok := proposalState.WithdrawableBudgets[budget.Stage]; !ok {
				unusedBudget += budget.Amount
			}
		}
	}

	history.Append(height, func() {
		proposalState.TrackingCount++
		switch trackingType {
		case payload.Common:
		case payload.Progress:
			proposalState.BudgetsStatus[proposalTracking.Stage] = Withdrawable
			for _, budget := range proposalState.Proposal.Budgets {
				if budget.Stage == proposalTracking.Stage {
					proposalState.WithdrawableBudgets[proposalTracking.Stage] = budget.Amount
					break
				}
			}
			if len(proposalState.WithdrawnBudgets) == len(proposalState.Proposal.Budgets)-1 {
				proposalState.FinalPaymentStatus = true
			}
		case payload.Rejected:
			proposalState.BudgetsStatus[proposalTracking.Stage] = Rejected
		case payload.ChangeOwner:
			proposalState.ProposalOwner = proposalTracking.NewOwnerPublicKey
		case payload.Terminated:
			proposalState.TerminatedHeight = height
			proposalState.Status = Terminated
			for k, v := range proposalState.BudgetsStatus {
				if v == Unfinished || v == Rejected {
					proposalState.BudgetsStatus[k] = Closed
				}
			}
		case payload.Finalized:
			proposalState.Status = Finished
			for _, budget := range proposalState.Proposal.Budgets {
				if budget.Type == payload.FinalPayment {
					proposalState.WithdrawableBudgets[budget.Stage] = budget.Amount
					break
				}
			}
			proposalState.BudgetsStatus[proposalTracking.Stage] = Withdrawable
			for k, v := range proposalState.BudgetsStatus {
				if v == Unfinished || v == Rejected {
					proposalState.BudgetsStatus[k] = Closed
				}
			}
		}
	}, func() {
		proposalState.TrackingCount--
		switch trackingType {
		case payload.Common:
		case payload.Progress:
			delete(proposalState.WithdrawableBudgets, proposalTracking.Stage)
			proposalState.BudgetsStatus = oriBudgetsStatus
			proposalState.FinalPaymentStatus = false
		case payload.Rejected:
			proposalState.BudgetsStatus = oriBudgetsStatus
		case payload.ChangeOwner:
			proposalState.ProposalOwner = owner
		case payload.Terminated:
			proposalState.BudgetsStatus = oriBudgetsStatus
			proposalState.TerminatedHeight = terminatedHeight
			proposalState.Status = status
		case payload.Finalized:
			proposalState.BudgetsStatus = oriBudgetsStatus
			proposalState.Status = status
			for _, budget := range proposalState.Proposal.Budgets {
				if budget.Type == payload.FinalPayment {
					delete(proposalState.WithdrawableBudgets, budget.Stage)
					break
				}
			}
		}
	})

	return
}

func NewProposalManager(params *config.Params) *ProposalManager {
	return &ProposalManager{
		ProposalKeyFrame: *NewProposalKeyFrame(),
		params:           params,
		history:          utils.NewHistory(maxHistoryCapacity),
	}
}
