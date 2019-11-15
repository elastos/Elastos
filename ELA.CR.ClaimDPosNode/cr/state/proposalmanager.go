// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"fmt"
	"sync"

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
	params *config.Params
	mtx    sync.Mutex
}

// ExistDraft judge if specified draft (that related to a proposal) exist.
func (p *ProposalManager) ExistDraft(hash common.Uint256) bool {
	p.mtx.Lock()
	defer p.mtx.Unlock()

	for _, v := range p.Proposals {
		if v.Proposal.DraftHash.IsEqual(hash) {
			return true
		}
	}
	return false
}

// ExistProposal judge if specified proposal exist.
func (p *ProposalManager) ExistProposal(hash common.Uint256) bool {
	p.mtx.Lock()
	defer p.mtx.Unlock()

	_, ok := p.Proposals[hash]
	return ok
}

func (p *ProposalManager) GetAllProposals() (dst ProposalsMap) {
	p.mtx.Lock()
	defer p.mtx.Unlock()
	return p.getAllProposal()
}
func (p *ProposalManager) getAllProposal() (dst ProposalsMap) {
	dst = NewProposalMap()
	for k, v := range p.Proposals {
		p := *v
		dst[k] = &p
	}
	return
}

func (p *ProposalManager) GetProposals(status ProposalStatus) (dst ProposalsMap) {
	p.mtx.Lock()
	defer p.mtx.Unlock()
	return p.getProposals(status)
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

func (p *ProposalManager) GetProposalByDraftHash(draftHash common.
	Uint256) *ProposalState {
	p.mtx.Lock()
	defer p.mtx.Unlock()

	return p.getProposalByDraftHash(draftHash)
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

// GetProposal will return a proposal with specified hash,
// and return nil if not found.
func (p *ProposalManager) GetProposal(hash common.Uint256) *ProposalState {
	p.mtx.Lock()
	defer p.mtx.Unlock()

	return p.getProposal(hash)
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

// updateProposals will update proposals' status.
func (p *ProposalManager) updateProposals(height uint32,
	circulation common.Fixed64, history *utils.History) {
	for k, v := range p.Proposals {
		switch v.Status {
		case Registered:
			if p.shouldEndCRCVote(k, height) {
				p.transferRegisteredState(v, height, history)
			}
		case CRAgreed:
			if p.shouldEndPublicVote(k, height) {
				p.transferCRAgreedState(v, height, circulation, history)
			}
		}
	}
}

// transferRegisteredState will transfer the Registered state by CR agreement
// count.
func (p *ProposalManager) transferRegisteredState(proposal *ProposalState,
	height uint32, history *utils.History) {
	agreedCount := uint32(0)
	for _, v := range proposal.CRVotes {
		if v == payload.Approve {
			agreedCount++
		}
	}

	oriVoteStartHeight := proposal.VoteStartHeight

	if agreedCount >= p.params.CRAgreementCount {
		history.Append(height, func() {
			proposal.Status = CRAgreed
			proposal.VoteStartHeight = height
		}, func() {
			proposal.Status = Registered
			proposal.VoteStartHeight = oriVoteStartHeight
		})
	} else {
		history.Append(height, func() {
			proposal.Status = CRCanceled
		}, func() {
			proposal.Status = Registered
		})
	}
}

func (p *ProposalManager) AvailableWithdrawalAmount(hash common.Uint256) common.Fixed64 {
	p.mtx.Lock()
	defer p.mtx.Unlock()
	propState := p.getProposal(hash)
	if propState == nil {
		return common.Fixed64(0)
	}
	amout := common.Fixed64(0)

	//Budgets slice index from 0---n-1
	//stage   user  index from 1---n
	start := propState.CurrentWithdrawalStage
	end := propState.CurrentStage
	for i := start; i < end; i++ {
		amout += propState.Proposal.Budgets[i]
	}
	return amout
}

// transferCRAgreedState will transfer CRAgreed state by votes' reject amount.
func (p *ProposalManager) transferCRAgreedState(proposal *ProposalState,
	height uint32, circulation common.Fixed64, history *utils.History) {
	if proposal.VotersRejectAmount >= common.Fixed64(float64(circulation)*
		p.params.VoterRejectPercentage/100.0) {
		history.Append(height, func() {
			proposal.Status = VoterCanceled
		}, func() {
			proposal.Status = CRAgreed
		})
	} else {
		history.Append(height, func() {
			proposal.Status = VoterAgreed
			proposal.CurrentStage = 1
		}, func() {
			proposal.Status = CRAgreed
			proposal.CurrentStage = 0
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

func (p *ProposalManager) IsProposalFull(did common.Uint168) bool {
	p.mtx.Lock()
	defer p.mtx.Unlock()
	return p.isProposalFull(did)
}

func (p *ProposalManager) isProposalFull(did common.Uint168) bool {
	return p.getProposalCount(did) >= int(p.params.MaxCommitteeProposalCount)
}

func (p *ProposalManager) GetProposalCount(did common.Uint168) int {
	p.mtx.Lock()
	defer p.mtx.Unlock()
	return p.getProposalCount(did)
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
		Status:                 Registered,
		Proposal:               *proposal,
		TxHash:                 tx.Hash(),
		RegisterHeight:         height,
		CRVotes:                map[common.Uint168]payload.VoteResult{},
		VotersRejectAmount:     common.Fixed64(0),
		CurrentStage:           0,
		CurrentWithdrawalStage: 0,
		ProposalLeader:         proposal.SponsorPublicKey,
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
	withdrawStage := proposalState.CurrentWithdrawalStage
	history.Append(height, func() {
		proposalState.CurrentWithdrawalStage = withdrawPayload.Stage
	}, func() {
		proposalState.CurrentWithdrawalStage = withdrawStage
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
	stage := proposalTracking.Stage
	status := proposalState.Status

	history.Append(height, func() {
		proposalState.TrackingCount++
		switch trackingType {
		case payload.Common:
		case payload.Progress:
		case payload.Terminated:
			proposalState.TerminatedHeight = height
			proposalState.Status = Aborted
		case payload.ProposalLeader:
			proposalState.ProposalLeader = proposalTracking.NewLeaderPubKey
		case payload.Appropriation:
			proposalState.CurrentStage = proposalTracking.Stage
			if int(proposalTracking.Stage) == len(proposalState.Proposal.Budgets) {
				proposalState.Status = Finished
			}
		}
	}, func() {
		proposalState.TrackingCount--
		switch trackingType {
		case payload.Common:
		case payload.Progress:
		case payload.Terminated:
			proposalState.TerminatedHeight = terminatedHeight
			proposalState.Status = status
		case payload.ProposalLeader:
			proposalState.ProposalLeader = leader
		case payload.Appropriation:
			proposalState.CurrentStage = stage
			proposalState.Status = status
		}
	})
}

func NewProposalManager(params *config.Params) *ProposalManager {
	return &ProposalManager{
		params:           params,
		ProposalKeyFrame: *NewProposalKeyFrame(),
	}
}
