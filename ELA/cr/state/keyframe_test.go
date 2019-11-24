// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
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
	if len(first.Nicknames) != len(second.Nicknames) ||
		len(first.CodeDIDMap) != len(second.CodeDIDMap) ||
		len(first.Votes) != len(second.Votes) {
		return false
	}

	for k := range first.Nicknames {
		if _, ok := second.Nicknames[k]; !ok {
			return false
		}
	}

	for k, v := range first.CodeDIDMap {
		v2, ok := second.CodeDIDMap[k]
		if !ok {
			return false
		}

		if !v.IsEqual(v2) {
			return false
		}
	}

	for k := range first.Votes {
		_, ok := second.Votes[k]
		if !ok {
			return false
		}
	}

	return candidatesMapEqual(first.Candidates, second.Candidates)

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

func keyframeEqual(first *KeyFrame, second *KeyFrame) bool {
	if first.LastCommitteeHeight != second.LastCommitteeHeight ||
		len(first.Members) != len(second.Members) {
		return false
	}

	for k, v := range first.Members {
		if !crMemberEqual(v, second.Members[k]) {
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
		first.ImpeachmentVotes == second.ImpeachmentVotes &&
		first.Penalty == second.Penalty
}

func randomCRMember() *CRMember {
	return &CRMember{
		Info:             *randomCRInfo(),
		ImpeachmentVotes: common.Fixed64(rand.Uint64()),
	}
}

func randomStateKeyFrame(size int, hasPending bool) *StateKeyFrame {
	frame := NewStateKeyFrame()

	for i := 0; i < size; i++ {
		candidate := randomCandidate()
		candidate.state = Pending
		nickname := candidate.Info().NickName
		code := candidate.Info().Code
		did := candidate.Info().DID
		frame.CodeDIDMap[common.BytesToHexString(code)] = did
		frame.Candidates[did] = candidate
		frame.Nicknames[nickname] = struct{}{}
	}
	if hasPending {
		for i := 0; i < size; i++ {
			candidate := randomCandidate()
			candidate.state = Active
			code := candidate.info.Code
			did := candidate.info.DID
			nickname := candidate.info.NickName
			frame.CodeDIDMap[common.BytesToHexString(code)] = did
			frame.Candidates[did] = candidate
			frame.Nicknames[nickname] = struct{}{}
		}
	}
	for i := 0; i < size; i++ {

		candidate := randomCandidate()

		did := candidate.info.DID
		nickname := candidate.info.NickName
		code := candidate.info.Code
		if i%2 == 0 {
			candidate.state = Canceled
		} else {
			candidate.state = Returned
		}
		frame.CodeDIDMap[common.BytesToHexString(code)] = did
		frame.Candidates[did] = candidate
		frame.Nicknames[nickname] = struct{}{}
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
			!bytes.Equal(v.Proposal.CRSign, proposalState.Proposal.CRSign) ||
			!bytes.Equal(v.Proposal.Sign, proposalState.Proposal.Sign) ||
			!bytes.Equal(v.Proposal.SponsorPublicKey, proposalState.Proposal.SponsorPublicKey) ||
			!v.Proposal.CRSponsorDID.IsEqual(proposalState.Proposal.CRSponsorDID) {
			return false
		}

		for i := range v.Proposal.Budgets {
			if v.Proposal.Budgets[i] != proposalState.Proposal.Budgets[i] {
				return false
			}
		}
	}

	return proposalHashEqual(first.ProposalHashs, second.ProposalHashs)
}

func randomProposalKeyframe() *ProposalKeyFrame {
	return &ProposalKeyFrame{Proposals: map[common.Uint256]*ProposalState{
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
		*randomUint256(): randomProposalState(),
	},
		ProposalHashs: map[common.Uint168]ProposalHashSet{
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
		ProposalType:     payload.CRCProposalType(rand.Int31n(6)),
		SponsorPublicKey: randomBytes(33),
		CRSponsorDID:     *randomUint168(),
		DraftHash:        *randomUint256(),
		Budgets: []common.Fixed64{
			common.Fixed64(rand.Int63()),
			common.Fixed64(rand.Int63()),
			common.Fixed64(rand.Int63()),
			common.Fixed64(rand.Int63()),
			common.Fixed64(rand.Int63()),
		},
		Sign:   randomBytes(64),
		CRSign: randomBytes(64),
	}
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
