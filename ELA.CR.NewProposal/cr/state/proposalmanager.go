// Copyright (c) 2017-2019 The Elastos Foundation
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

	// Aborted means proposal had been approved by both CR and voters,
	// whoever the proposal related project has been decided to abort for
	// some reason.
	Aborted ProposalStatus = 0x06
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

func (p *ProposalManager) getProposalByDraftHash(draftHash common.
	Uint256) *ProposalState {
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

// updateProposals will update proposals' status.
func (p *ProposalManager) updateProposals(height uint32,
	circulation common.Fixed64) {
	for k, v := range p.Proposals {
		switch v.Status {
		case Registered:
			if p.shouldEndCRCVote(k, height) {
				p.transferRegisteredState(v, height)
			}
		case CRAgreed:
			if p.shouldEndPublicVote(k, height) {
				p.transferCRAgreedState(v, height, circulation)
			}
		}
	}
	p.history.Commit(height)
}

// transferRegisteredState will transfer the Registered state by CR agreement
// count.
func (p *ProposalManager) transferRegisteredState(proposal *ProposalState,
	height uint32) {
	agreedCount := uint32(0)
	for _, v := range proposal.CRVotes {
		if v == payload.Approve {
			agreedCount++
		}
	}

	oriVoteStartHeight := proposal.VoteStartHeight

	if agreedCount >= p.params.CRAgreementCount {
		p.history.Append(height, func() {
			proposal.Status = CRAgreed
			proposal.VoteStartHeight = height
		}, func() {
			proposal.Status = Registered
			proposal.VoteStartHeight = oriVoteStartHeight
		})
	} else {
		p.history.Append(height, func() {
			proposal.Status = CRCanceled
		}, func() {
			proposal.Status = Registered
		})
	}
}

// transferCRAgreedState will transfer CRAgreed state by votes' reject amount.
func (p *ProposalManager) transferCRAgreedState(proposal *ProposalState,
	height uint32, circulation common.Fixed64) {
	if proposal.VotersRejectAmount >= common.Fixed64(float64(circulation)*
		p.params.VoterRejectPercentage/100.0) {
		p.history.Append(height, func() {
			proposal.Status = VoterCanceled
		}, func() {
			proposal.Status = CRAgreed
		})
	} else {
		p.history.Append(height, func() {
			proposal.Status = VoterAgreed
			for _, b := range proposal.Proposal.Budgets {
				if b.Type == payload.Imprest {
					proposal.WithdrawableBudgets[b.Stage] = b.Amount
				}
			}
		}, func() {
			proposal.Status = CRAgreed
			for _, b := range proposal.Proposal.Budgets {
				if b.Type == payload.Imprest {
					delete(proposal.WithdrawableBudgets, b.Stage)
				}
			}

		})
	}
}

// shouldEndCRCVote returns if current height should end CRC vote about
// 	the specified proposal.
func (p *ProposalManager) shouldEndCRCVote(hash common.Uint256,
	height uint32) bool {
	proposal := p.getProposal(hash)
	if proposal == nil {
		return false
	}
	return proposal.RegisterHeight+p.params.ProposalCRVotingPeriod <= height
}

// shouldEndPublicVote returns if current height should end public vote
// about the specified proposal.
func (p *ProposalManager) shouldEndPublicVote(hash common.Uint256,
	height uint32) bool {
	proposal := p.getProposal(hash)
	if proposal == nil {
		return false
	}
	return proposal.VoteStartHeight+p.params.ProposalPublicVotingPeriod <=
		height
}

func (p *ProposalManager) isProposalFull(did common.Uint168) bool {
	return p.getProposalCount(did) >= int(p.params.MaxCommitteeProposalCount)
}

func (p *ProposalManager) getProposalCount(did common.Uint168) int {
	proposalHashsSet, ok := p.ProposalHashs[did]
	if !ok {
		return 0
	}
	return proposalHashsSet.Len()
}

func (p *ProposalManager) addProposal(did common.Uint168,
	proposalHash common.Uint256) {
	proposalHashsSet, ok := p.ProposalHashs[did]
	if !ok {
		proposalHashsSet = NewProposalHashSet()
		proposalHashsSet.Add(proposalHash)
		p.ProposalHashs[did] = proposalHashsSet
		return
	}
	proposalHashsSet.Add(proposalHash)
}

func (p *ProposalManager) delProposal(did common.Uint168,
	proposalHash common.Uint256) {
	proposalHashsSet, ok := p.ProposalHashs[did]
	if ok {
		proposalHashsSet.Remove(proposalHash)
	}
}

// registerProposal will register proposal state in proposal manager
func (p *ProposalManager) registerProposal(tx *types.Transaction,
	height uint32, history *utils.History) {
	proposal := tx.Payload.(*payload.CRCProposal)
	//The number of the proposals of the committee can not more than 128
	if p.isProposalFull(proposal.CRSponsorDID) {
		return
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
		FinalPaymentStatus:  false,
		TrackingCount:       0,
		TerminatedHeight:    0,
		ProposalLeader:      proposal.SponsorPublicKey,
		AppropriatedStage:   0,
	}
	CRSponsorDID := proposal.CRSponsorDID
	hash := proposal.Hash()

	history.Append(height, func() {
		p.Proposals[proposal.Hash()] = proposalState
		p.addProposal(CRSponsorDID, hash)
	}, func() {
		delete(p.Proposals, proposal.Hash())
		p.delProposal(CRSponsorDID, hash)
	})
}

func getDIDByCode(code []byte) (*common.Uint168, error) {
	ct1, error := contract.CreateCRDIDContractByCode(code)
	if error != nil {
		return nil, error
	}
	return ct1.ToProgramHash(), error
}

func (p *ProposalManager) proposalReview(tx *types.Transaction,
	height uint32, history *utils.History) {
	proposalReview := tx.Payload.(*payload.CRCProposalReview)
	proposalState := p.getProposal(proposalReview.ProposalHash)
	if proposalState == nil {
		return
	}
	did := proposalReview.DID
	history.Append(height, func() {
		proposalState.CRVotes[did] = proposalReview.VoteResult
	}, func() {
		delete(proposalState.CRVotes, did)
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
	history.Append(height, func() {
		for k, v := range withdrawingBudgets {
			proposalState.WithdrawnBudgets[k] = v
		}
	}, func() {
		for k, _ := range withdrawingBudgets {
			delete(proposalState.WithdrawnBudgets, k)
		}
	})
}

func (p *ProposalManager) proposalTracking(tx *types.Transaction,
	height uint32, history *utils.History) {
	proposalTracking := tx.Payload.(*payload.CRCProposalTracking)
	proposalState := p.getProposal(proposalTracking.ProposalHash)
	if proposalState == nil {
		return
	}

	trackingType := proposalTracking.ProposalTrackingType
	leader := proposalState.ProposalLeader
	terminatedHeight := proposalState.TerminatedHeight
	status := proposalState.Status

	history.Append(height, func() {
		proposalState.TrackingCount++
		switch trackingType {
		case payload.Common:
		case payload.Progress:
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
		case payload.ProposalLeader:
			proposalState.ProposalLeader = proposalTracking.NewLeaderPubKey
		case payload.Terminated:
			proposalState.TerminatedHeight = height
			proposalState.Status = Aborted
		case payload.Finalized:
			if int(proposalTracking.Stage) == len(proposalState.Proposal.Budgets) {
				proposalState.Status = Finished
			}
		}
	}, func() {
		proposalState.TrackingCount--
		switch trackingType {
		case payload.Common:
		case payload.Progress:
			delete(proposalState.WithdrawableBudgets, proposalTracking.Stage)
			proposalState.FinalPaymentStatus = false
		case payload.Rejected:
		case payload.ProposalLeader:
			proposalState.ProposalLeader = leader
		case payload.Terminated:
			proposalState.TerminatedHeight = terminatedHeight
			proposalState.Status = status
		case payload.Finalized:
			proposalState.Status = status
		}
	})
}

func NewProposalManager(params *config.Params) *ProposalManager {
	return &ProposalManager{
		ProposalKeyFrame: *NewProposalKeyFrame(),
		params:           params,
		history:          utils.NewHistory(maxHistoryCapacity),
	}
}
