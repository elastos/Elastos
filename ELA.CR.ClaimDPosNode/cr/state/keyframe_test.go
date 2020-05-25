// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package state

import (
	"bytes"
	crand "crypto/rand"
	"encoding/binary"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/assert"
)

func TestKeyFrame_Deserialize(t *testing.T) {
	frame := randomKeyFrame(5, rand.Uint32())

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &KeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, keyframeEqual(frame, frame2))
}

func TestKeyFrame_Snapshot(t *testing.T) {
	frame := randomKeyFrame(5, rand.Uint32())
	frame2 := frame.Snapshot()
	assert.True(t, keyframeEqual(frame, frame2))
}

func TestStateKeyFrame_Deserialize(t *testing.T) {
	frame := randomStateKeyFrame(5, true)

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &StateKeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, stateKeyframeEqual(frame, frame2))
}

func TestStateKeyFrame_Snapshot(t *testing.T) {
	frame := randomStateKeyFrame(5, true)
	frame2 := frame.Snapshot()
	assert.True(t, stateKeyframeEqual(frame, frame2))
}

func TestProposalKeyFrame_Deserialize(t *testing.T) {
	frame := randomProposalKeyframe()

	buf := new(bytes.Buffer)
	frame.Serialize(buf)

	frame2 := &ProposalKeyFrame{}
	frame2.Deserialize(buf)

	assert.True(t, proposalKeyFrameEqual(frame, frame2))
}

func TestProposalKeyFrame_Snapshot(t *testing.T) {
	frame := randomProposalKeyframe()
	frame2 := frame.Snapshot()
	assert.True(t, proposalKeyFrameEqual(frame, frame2))
}

func stateKeyframeEqual(first *StateKeyFrame, second *StateKeyFrame) bool {
	if first.CurrentSession != second.CurrentSession ||
		len(first.Candidates) != len(second.Candidates) ||
		len(first.HistoryCandidates) != len(second.HistoryCandidates) ||
		len(first.DepositHashCIDMap) != len(second.DepositHashCIDMap) ||
		len(first.CodeCIDMap) != len(second.CodeCIDMap) ||
		len(first.DepositOutputs) != len(second.DepositOutputs) ||
		len(first.CRCFoundationOutputs) != len(second.CRCFoundationOutputs) ||
		len(first.CRCCommitteeOutputs) != len(second.CRCCommitteeOutputs) ||
		len(first.Nicknames) != len(second.Nicknames) ||
		len(first.Votes) != len(second.Votes) {
		return false
	}

	return candidatesMapEqual(first.Candidates, second.Candidates) &&
		candidatesHistoryMapEqual(first.HistoryCandidates, second.HistoryCandidates) &&
		depositHashCIDMapEqual(first.DepositHashCIDMap, second.DepositHashCIDMap) &&
		codeCIDMapEqual(first.CodeCIDMap, second.CodeCIDMap) &&
		amountMapEqual(first.DepositOutputs, second.DepositOutputs) &&
		amountMapEqual(first.CRCFoundationOutputs, second.CRCFoundationOutputs) &&
		amountMapEqual(first.CRCCommitteeOutputs, second.CRCCommitteeOutputs) &&
		stringMapEqual(first.Nicknames, second.Nicknames) &&
		stringMapEqual(first.Votes, second.Votes)

}

func stringMapEqual(first map[string]struct{}, second map[string]struct{}) bool {
	for k := range first {
		if _, ok := second[k]; !ok {
			return false
		}
	}
	return true
}

func codeCIDMapEqual(first map[string]common.Uint168,
	second map[string]common.Uint168) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		v2, ok := second[k]
		if !ok {
			return false
		}

		if !v.IsEqual(v2) {
			return false
		}
	}
	return true
}

func depositHashCIDMapEqual(first map[common.Uint168]common.Uint168,
	second map[common.Uint168]common.Uint168) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		v2, ok := second[k]
		if !ok {
			return false
		}

		if !v.IsEqual(v2) {
			return false
		}
	}
	return true
}

func amountMapEqual(first map[string]common.Fixed64,
	second map[string]common.Fixed64) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		v2, ok := second[k]
		if !ok {
			return false
		}

		if v != v2 {
			return false
		}
	}
	return true
}

func candidatesMapEqual(first map[common.Uint168]*Candidate,
	second map[common.Uint168]*Candidate) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		v2, ok := second[k]
		if !ok {
			return false
		}

		if !candidateEqual(v, v2) {
			return false
		}
	}
	return true
}

func candidatesHistoryMapEqual(first map[uint64]map[common.Uint168]*Candidate,
	second map[uint64]map[common.Uint168]*Candidate) bool {
	if len(first) != len(second) {
		return false
	}
	for k, v := range first {
		if !candidatesMapEqual(v, second[k]) {
			return false
		}
	}
	return true
}

func keyframeEqual(first *KeyFrame, second *KeyFrame) bool {
	if first.LastCommitteeHeight != second.LastCommitteeHeight ||
		first.LastVotingStartHeight != second.LastVotingStartHeight ||
		first.InElectionPeriod != second.InElectionPeriod ||
		first.NeedAppropriation != second.NeedAppropriation ||
		first.CRCFoundationBalance != second.CRCFoundationBalance ||
		first.CRCCommitteeBalance != second.CRCCommitteeBalance ||
		first.CRCCommitteeUsedAmount != second.CRCCommitteeUsedAmount ||
		first.DestroyedAmount != second.DestroyedAmount ||
		first.CirculationAmount != second.CirculationAmount ||
		len(first.Members) != len(second.Members) ||
		len(first.HistoryMembers) != len(second.HistoryMembers) {
		return false
	}

	if !membersEuqal(first.Members, second.Members) {
		return false
	}

	for k, v := range first.HistoryMembers {
		if !membersEuqal(v, second.HistoryMembers[k]) {
			return false
		}
	}

	return true
}

func membersEuqal(first map[common.Uint168]*CRMember,
	second map[common.Uint168]*CRMember) bool {
	for k, v := range first {
		if !crMemberEqual(v, second[k]) {
			return false
		}
	}
	return true
}

func randomKeyFrame(size int, commitHeight uint32) *KeyFrame {
	frame := &KeyFrame{
		LastCommitteeHeight: commitHeight,
	}

	frame.Members = make(map[common.Uint168]*CRMember, size)
	for i := 0; i < size; i++ {
		m := randomCRMember()
		frame.Members[m.Info.DID] = m
	}

	return frame
}

func crMemberEqual(first *CRMember, second *CRMember) bool {
	return crInfoEqual(&first.Info, &second.Info) &&
		first.ImpeachmentVotes == second.ImpeachmentVotes
}

func randomCRMember() *CRMember {
	return &CRMember{
		Info:             *randomCRInfo(),
		ImpeachmentVotes: common.Fixed64(rand.Uint64()),
	}
}

func randomStateKeyFrame(size int, hasPending bool) *StateKeyFrame {
	frame := NewStateKeyFrame()

	if hasPending {
		for i := 0; i < size; i++ {
			candidate := randomCandidate()
			candidate.state = Pending
			nickname := candidate.Info().NickName
			code := candidate.Info().Code
			cid := candidate.Info().CID
			frame.CodeCIDMap[common.BytesToHexString(code)] = cid
			frame.Candidates[cid] = candidate
			frame.Nicknames[nickname] = struct{}{}
			frame.depositInfo[cid] = &DepositInfo{
				Refundable:    false,
				DepositAmount: 5000 * 1e8,
				TotalAmount:   5000 * 1e8,
			}
		}
	}
	for i := 0; i < size; i++ {
		candidate := randomCandidate()
		candidate.state = Active
		code := candidate.info.Code
		cid := candidate.info.CID
		nickname := candidate.info.NickName
		frame.CodeCIDMap[common.BytesToHexString(code)] = cid
		frame.Candidates[cid] = candidate
		frame.Nicknames[nickname] = struct{}{}
		frame.depositInfo[cid] = &DepositInfo{
			Refundable:    false,
			DepositAmount: 5000 * 1e8,
			TotalAmount:   5000 * 1e8,
		}
	}
	for i := 0; i < size; i++ {
		candidate := randomCandidate()
		cid := candidate.info.CID

		nickname := candidate.info.NickName
		code := candidate.info.Code
		var refundable bool
		if i%2 == 0 {
			candidate.state = Canceled
			refundable = true
		} else {
			candidate.state = Returned
			refundable = false
		}
		frame.CodeCIDMap[common.BytesToHexString(code)] = cid
		frame.Candidates[cid] = candidate
		frame.Nicknames[nickname] = struct{}{}
		frame.depositInfo[cid] = &DepositInfo{
			Refundable:    refundable,
			DepositAmount: 5000 * 1e8,
			TotalAmount:   5000 * 1e8,
		}
	}
	frame.HistoryCandidates[1] = make(map[common.Uint168]*Candidate)
	for i := 0; i < size; i++ {
		candidate := randomCandidate()
		frame.depositInfo[candidate.info.DID] = &DepositInfo{}
		frame.HistoryCandidates[1][candidate.info.DID] = candidate
	}
	for i := 0; i < size; i++ {
		frame.Votes[randomString()] = struct{}{}
	}
	return frame
}

func proposalHashEqual(first, second map[common.Uint168]ProposalHashSet) bool {
	if len(first) != len(second) {
		return false
	}
	for firstK, firstV := range first {
		secondV, ok := second[firstK]
		if !ok {
			return false
		}

		if !firstV.Equal(secondV) {
			return false
		}
	}
	return true
}

func proposalKeyFrameEqual(first, second *ProposalKeyFrame) bool {
	if len(first.Proposals) != len(second.Proposals) {
		return false
	}
	for k, v := range first.Proposals {
		proposalState, exist := second.Proposals[k]
		if !exist {
			return false
		}

		if !v.TxHash.IsEqual(proposalState.TxHash) ||
			v.Status != proposalState.Status ||
			v.VotersRejectAmount != proposalState.VotersRejectAmount ||
			v.VoteStartHeight != proposalState.VoteStartHeight ||
			v.RegisterHeight != proposalState.RegisterHeight {
			return false
		}

		for k, v := range v.CRVotes {
			vote, ok := proposalState.CRVotes[k]
			if !ok {
				return false
			}
			if vote != v {
				return false
			}
		}

		if !v.Proposal.DraftHash.IsEqual(proposalState.Proposal.DraftHash) ||
			v.Proposal.ProposalType != proposalState.Proposal.ProposalType ||
			!bytes.Equal(v.Proposal.CRCouncilMemberSignature, proposalState.Proposal.CRCouncilMemberSignature) ||
			!bytes.Equal(v.Proposal.Signature, proposalState.Proposal.Signature) ||
			!bytes.Equal(v.Proposal.OwnerPublicKey, proposalState.Proposal.OwnerPublicKey) ||
			!v.Proposal.CRCouncilMemberDID.IsEqual(proposalState.Proposal.CRCouncilMemberDID) {
			return false
		}

		for i := range v.Proposal.Budgets {
			if v.Proposal.Budgets[i] != proposalState.Proposal.Budgets[i] {
				return false
			}
		}
	}

	return proposalHashEqual(first.ProposalHashes, second.ProposalHashes)
}

func randomProposalKeyframe() *ProposalKeyFrame {
	return &ProposalKeyFrame{Proposals: map[common.Uint256]*ProposalState{
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
	},
		ProposalHashes: map[common.Uint168]ProposalHashSet{
			*randomUint168(): randomProposalHashSet(),
			*randomUint168(): randomProposalHashSet(),
		},
	}
}

func randomProposalState() *ProposalState {
	return &ProposalState{
		Status:             ProposalStatus(rand.Int31n(7)),
		Proposal:           *randomCRCProposal(),
		TxHash:             *randomUint256(),
		RegisterHeight:     rand.Uint32(),
		VoteStartHeight:    rand.Uint32(),
		VotersRejectAmount: common.Fixed64(rand.Int63()),
		CRVotes: map[common.Uint168]payload.VoteResult{
			*randomUint168(): payload.VoteResult(rand.Int31n(3)),
			*randomUint168(): payload.VoteResult(rand.Int31n(3)),
			*randomUint168(): payload.VoteResult(rand.Int31n(3)),
			*randomUint168(): payload.VoteResult(rand.Int31n(3)),
			*randomUint168(): payload.VoteResult(rand.Int31n(3)),
		},
	}
}

func randomProposalHashSet() ProposalHashSet {
	proposalHashSet := NewProposalHashSet()
	count := rand.Int() % 128
	for i := 0; i < count; i++ {
		proposalHashSet.Add(*randomUint256())
	}

	return proposalHashSet
}

func randomCRCProposal() *payload.CRCProposal {
	return &payload.CRCProposal{
		ProposalType:             payload.CRCProposalType(rand.Int31n(6)),
		OwnerPublicKey:           randomBytes(33),
		CRCouncilMemberDID:       *randomUint168(),
		DraftHash:                *randomUint256(),
		Budgets:                  createBudgets(5),
		Signature:                randomBytes(64),
		CRCouncilMemberSignature: randomBytes(64),
	}
}

func createBudgets(n int) []payload.Budget {
	budgets := make([]payload.Budget, 0)
	for i := 0; i < n; i++ {
		var budgetType = payload.NormalPayment
		if i == 0 {
			budgetType = payload.Imprest
		}
		if i == n-1 {
			budgetType = payload.FinalPayment
		}
		budget := &payload.Budget{
			Stage:  byte(i),
			Type:   budgetType,
			Amount: common.Fixed64((i + 1) * 1e8),
		}
		budgets = append(budgets, *budget)
	}
	return budgets
}

func randomFix64() common.Fixed64 {
	var randNum int64
	binary.Read(crand.Reader, binary.BigEndian, &randNum)
	return common.Fixed64(randNum)
}

func randomOutputs() *types.Output {
	return &types.Output{
		AssetID:     *randomUint256(),
		Value:       common.Fixed64(rand.Int63()),
		OutputLock:  0,
		ProgramHash: *randomUint168(),
		Type:        types.OTVote,
		Payload: &outputpayload.VoteOutput{
			Version: outputpayload.VoteProducerAndCRVersion,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{
							Candidate: randomBytes(34),
							Votes:     common.Fixed64(rand.Int63()),
						},
					},
				},
			},
		},
	}
}
