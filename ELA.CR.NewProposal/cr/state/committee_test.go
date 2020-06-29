// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
	"sort"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"

	"github.com/stretchr/testify/assert"
)

func TestSortTransactions(t *testing.T) {
	txs := []*types.Transaction{
		&types.Transaction{TxType: types.CoinBase},
		&types.Transaction{TxType: types.TransferAsset},
		&types.Transaction{TxType: types.TransferAsset},
		&types.Transaction{TxType: types.CRCProposalTracking},
		&types.Transaction{TxType: types.CRCProposalWithdraw},
		&types.Transaction{TxType: types.CRCProposalWithdraw},
		&types.Transaction{TxType: types.TransferAsset},
		&types.Transaction{TxType: types.CRCProposalWithdraw},
	}

	sortTransactions(txs[1:])
	assert.Equal(t, txs[0].TxType.Name(), "CoinBase")
	assert.Equal(t, txs[1].TxType.Name(), "CRCProposalWithdraw")
	assert.Equal(t, txs[2].TxType.Name(), "CRCProposalWithdraw")
	assert.Equal(t, txs[3].TxType.Name(), "CRCProposalWithdraw")
	assert.Equal(t, txs[4].TxType.Name(), "TransferAsset")
	assert.Equal(t, txs[5].TxType.Name(), "TransferAsset")
	assert.Equal(t, txs[6].TxType.Name(), "CRCProposalTracking")
	assert.Equal(t, txs[7].TxType.Name(), "TransferAsset")
}

func TestNewCRCommittee(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)

	assert.Equal(t, uint32(0), committee.LastCommitteeHeight)
	assert.Equal(t, 0, len(committee.GetMembersCodes()))
	assert.Equal(t, 0, len(committee.GetMembersDIDs()))
}

func TestCommittee_ProcessBlock(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)
	round1, expectCandidates1, votes1 := generateCandidateSuite()
	round2, expectCandidates2, votes2 := generateCandidateSuite()
	committee.state.StateKeyFrame = *round1

	// < CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight - 1,
		},
	}, nil)
	assert.Equal(t, 0, len(committee.GetMembersCodes()))
	assert.Equal(t, 0, len(committee.GetMembersDIDs()))

	// CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight,
		},
	}, nil)
	codes1 := committee.GetMembersCodes()
	sortCodeList(codes1, votes1)
	did1 := committee.GetMembersDIDs()
	sortDIDList(did1, votes1)

	for i := 0; i < len(expectCandidates1); i++ {
		if i > 0 {
			assert.True(t,
				expectCandidates1[i-1].votes > expectCandidates1[i].votes)
		}
		assert.True(t, existCode(expectCandidates1[i].info.Code, codes1))
		assert.True(t, existID(expectCandidates1[i].info.DID, did1))
	}

	// > CRCommitteeStartHeight && < CRCommitteeStartHeight + CRDutyPeriod
	for k, v := range round1.depositInfo {
		round2.depositInfo[k] = v
	}
	committee.state.StateKeyFrame = *round2
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod - 1,
		},
	}, nil)
	codes2 := committee.GetMembersCodes()
	sortCodeList(codes2, votes1)
	did2 := committee.GetMembersDIDs()
	sortDIDList(did2, votes1)
	for i := 0; i < len(expectCandidates1); i++ {
		assert.True(t, existCode(expectCandidates1[i].info.Code, codes2))
		assert.True(t, existID(expectCandidates1[i].info.DID, did2))
	}

	// CRCommitteeStartHeight + CRDutyPeriod
	committee.LastVotingStartHeight = config.DefaultParams.CRCommitteeStartHeight +
		config.DefaultParams.CRDutyPeriod - config.DefaultParams.CRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod,
		},
	}, nil)
	codes2 = committee.GetMembersCodes()
	sortCodeList(codes2, votes2)
	did2 = committee.GetMembersDIDs()
	sortDIDList(did2, votes2)
	for i := 0; i < len(expectCandidates2); i++ {
		if i > 0 {
			assert.True(t,
				expectCandidates2[i-1].votes > expectCandidates2[i].votes)
		}
		assert.True(t, bytes.Equal(expectCandidates2[i].info.Code, codes2[i]))
		assert.True(t, expectCandidates2[i].info.DID.IsEqual(did2[i]))
	}
}

func sortCodeList(codes [][]byte, votes map[common.Uint168]common.Fixed64) {
	sort.Slice(codes, func(i, j int) bool {
		firstDID, _ := getDIDByCode(codes[i])
		secondDID, _ := getDIDByCode(codes[j])
		return votes[*firstDID] > votes[*secondDID]
	})
}

func sortDIDList(cid []common.Uint168, votes map[common.Uint168]common.Fixed64) {
	sort.Slice(cid, func(i, j int) bool {
		return votes[cid[i]] > votes[cid[j]]
	})
}

func TestCommittee_IsInVotingPeriod(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)

	// 0
	assert.False(t, committee.IsInVotingPeriod(0))

	// < CRVotingStartHeight
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRVotingStartHeight-1))

	// [CRVotingStartHeight, CRCommitteeStartHeight - CRVotingPeriod]
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight-
			config.DefaultParams.CRVotingPeriod-1))

	// CRCommitteeStartHeight - CRVotingPeriod
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight-
			config.DefaultParams.CRVotingPeriod))

	// [CRCommitteeStartHeight - CRVotingPeriod, CRCommitteeStartHeight)
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight-
			config.DefaultParams.CRVotingPeriod+1))

	// CRCommitteeStartHeight
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight))

	// change to first committee
	committee.LastCommitteeHeight = config.DefaultParams.CRCommitteeStartHeight

	// < CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod
	committee.InElectionPeriod = true
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod-config.DefaultParams.
			CRVotingPeriod-1))

	// CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod-config.DefaultParams.
			CRVotingPeriod))

	// [CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod,
	// CRCommitteeStartHeight + CRDutyPeriod)
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod-config.DefaultParams.
			CRVotingPeriod+1))

	// CRCommitteeStartHeight + CRDutyPeriod
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod))

	// change to next committee
	committee.LastCommitteeHeight = config.DefaultParams.
		CRCommitteeStartHeight + config.DefaultParams.CRDutyPeriod

	// < CRCommitteeStartHeight + CRDutyPeriod * 2 - CRVotingPeriod
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod*2-config.DefaultParams.
			CRVotingPeriod-1))

	// CRCommitteeStartHeight + CRDutyPeriod * 2 - CRVotingPeriod
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod*2-config.DefaultParams.
			CRVotingPeriod))

	// [CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod,
	// CRCommitteeStartHeight + CRDutyPeriod)
	assert.True(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod*2-config.DefaultParams.
			CRVotingPeriod+1))

	// CRCommitteeStartHeight + CRDutyPeriod * 2
	assert.False(t, committee.IsInVotingPeriod(
		config.DefaultParams.CRCommitteeStartHeight+
			config.DefaultParams.CRDutyPeriod*2))
}

func TestCommittee_RollbackTo_SameCommittee_VotingPeriod(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)
	committee.RegisterFuncitons(&CommitteeFuncsConfig{})

	code := randomBytes(34)
	nickname := randomString()
	cid := *randomUint168()

	// register candidate
	height := config.DefaultParams.CRCommitteeStartHeight -
		config.DefaultParams.CRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, cid, nickname),
		},
	}, nil)
	candidate := committee.GetCandidate(cid)
	assert.Equal(t, Pending, candidate.state)
	assert.True(t, committee.ExistCandidateByNickname(nickname))
	height++

	// update candidate
	nickname2 := randomString()
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateUpdateCR(code, cid, nickname2),
		},
	}, nil)
	assert.True(t, committee.ExistCandidateByNickname(nickname2))
	height++

	// change state of candidate from pending to active
	for i := 0; i < 4; i++ {
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	assert.Equal(t, Active, candidate.state)

	// rollback to pending state
	assert.NoError(t, committee.RollbackTo(height-2))
	assert.Equal(t, Pending, candidate.state)

	// rollback to the nickname before update
	assert.NoError(t, committee.RollbackTo(
		config.DefaultParams.CRCommitteeStartHeight-
			config.DefaultParams.CRVotingPeriod))
	assert.True(t, committee.ExistCandidateByNickname(nickname))
}

func TestCommittee_RollbackTo_SameCommittee_BeforeVoting(t *testing.T) {
	//let committee be the first committee started state
	keyframe := randomKeyFrame(12,
		config.DefaultParams.CRCommitteeStartHeight)
	committee := NewCommittee(&config.DefaultParams)
	committee.KeyFrame = *keyframe
	committee.RegisterFuncitons(&CommitteeFuncsConfig{})

	// let processing height be 6 blocks before the second voting
	height := config.DefaultParams.CRCommitteeStartHeight + config.
		DefaultParams.CRDutyPeriod - config.DefaultParams.CRVotingPeriod - 6

	// simulate processing register CR before and after CR
	for i := 0; i < 10; i++ {
		code := randomBytes(34)
		nickname := randomString()
		cid := *randomUint168()

		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{
				generateRegisterCR(code, cid, nickname),
			},
		}, nil)
		height++
	}
	assert.False(t, keyframeEqual(keyframe, &committee.KeyFrame))
	assert.Equal(t, 5, len(committee.GetCandidates(Active)))

	// rollback within voting period, candidates' state changes but committee
	// state stay the same
	height -= 2
	assert.NoError(t, committee.RollbackTo(height))
	assert.False(t, keyframeEqual(keyframe, &committee.KeyFrame))
	assert.Equal(t, 4, len(committee.GetCandidates(Active)))

	// rollback to the voting height
	height = config.DefaultParams.CRCommitteeStartHeight + config.
		DefaultParams.CRDutyPeriod - config.DefaultParams.CRVotingPeriod
	assert.NoError(t, committee.RollbackTo(height))
	assert.False(t, keyframeEqual(keyframe, &committee.KeyFrame))
	assert.Equal(t, 2, len(committee.GetCandidates(Active)))

	// rollback to the height before voting
	height--
	assert.NoError(t, committee.RollbackTo(height))
	assert.False(t, keyframeEqual(keyframe, &committee.KeyFrame))
	assert.Equal(t, 1, len(committee.GetCandidates(Active)))

	// rollback to the height having no history
	height--
	assert.NoError(t, committee.RollbackTo(height))
	assert.False(t, keyframeEqual(keyframe, &committee.KeyFrame))
	assert.Equal(t, 0, len(committee.GetCandidates(Active)))
}

func TestCommittee_RollbackTo_DifferenceCommittee(t *testing.T) {
	//todo complete me when check point is done
}

func generateCandidateSuite() (*StateKeyFrame, []*Candidate, map[common.Uint168]common.Fixed64) {
	keyFrame := randomStateKeyFrame(24, false)
	candidates := make([]*Candidate, 0, 24)
	for _, v := range keyFrame.Candidates {
		if v.state == Active {
			candidates = append(candidates, v)
		}
	}
	sort.Slice(candidates, func(i, j int) bool {
		return candidates[i].votes > candidates[j].votes
	})

	topCandidates := make([]*Candidate, 0, 12)
	for i, v := range candidates {
		if i >= 12 {
			break
		}
		topCandidates = append(topCandidates, v)
	}

	votes := make(map[common.Uint168]common.Fixed64)
	for _, c := range candidates {
		votes[c.info.DID] = c.votes
	}

	return keyFrame, topCandidates, votes
}

func existCode(code []byte, codeArray [][]byte) bool {
	for _, v := range codeArray {
		if bytes.Equal(code, v) {
			return true
		}
	}
	return false
}

func existID(id common.Uint168, idArray []common.Uint168) bool {
	for _, v := range idArray {
		if v.IsEqual(id) {
			return true
		}
	}
	return false
}
