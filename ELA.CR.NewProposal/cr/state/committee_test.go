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

func TestNewCRCommittee(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)

	assert.Equal(t, uint32(0), committee.LastCommitteeHeight)
	assert.Equal(t, 0, len(committee.GetMembersCodes()))
	assert.Equal(t, 0, len(committee.GetMembersDIDs()))
}

func TestCommittee_ProcessBlock(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)
	round1, expectCandidates1 := generateCandidateSuite()
	round2, expectCandidates2 := generateCandidateSuite()
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
	did1 := committee.GetMembersDIDs()

	for i := 0; i < len(expectCandidates1); i++ {
		if i > 0 {
			assert.True(t,
				expectCandidates1[i-1].votes > expectCandidates1[i].votes)
		}
		assert.True(t, existCode(expectCandidates1[i].info.Code, codes1))
		assert.True(t, existDID(expectCandidates1[i].info.DID, did1))
	}

	// > CRCommitteeStartHeight && < CRCommitteeStartHeight + CRDutyPeriod
	committee.state.StateKeyFrame = *round2
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod - 1,
		},
	}, nil)
	codes2 := committee.GetMembersCodes()
	did2 := committee.GetMembersDIDs()
	for i := 0; i < len(expectCandidates1); i++ {
		assert.True(t, existCode(expectCandidates1[i].info.Code, codes2))
		assert.True(t, existDID(expectCandidates1[i].info.DID, did2))
	}

	// CRCommitteeStartHeight + CRDutyPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod,
		},
	}, nil)
	codes2 = committee.GetMembersCodes()
	did2 = committee.GetMembersDIDs()
	for i := 0; i < len(expectCandidates2); i++ {
		if i > 0 {
			assert.True(t,
				expectCandidates2[i-1].votes > expectCandidates2[i].votes)
		}
		assert.True(t, bytes.Equal(expectCandidates2[i].info.Code, codes2[i]))
		assert.True(t, expectCandidates2[i].info.DID.IsEqual(did2[i]))
	}
}

func TestCommittee_isInVotingPeriod(t *testing.T) {
	committee := NewCommittee(&config.DefaultParams)

	// 0
	assert.False(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: 0,
		},
	}))

	// < CRCommitteeStartHeight - CRVotingPeriod
	assert.False(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight -
				config.DefaultParams.CRVotingPeriod - 1,
		},
	}))

	// CRCommitteeStartHeight - CRVotingPeriod
	assert.True(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight -
				config.DefaultParams.CRVotingPeriod,
		},
	}))

	// [CRCommitteeStartHeight - CRVotingPeriod, CRCommitteeStartHeight)
	assert.True(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight -
				config.DefaultParams.CRVotingPeriod + 1,
		},
	}))

	// CRCommitteeStartHeight
	assert.False(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight,
		},
	}))

	committee.LastCommitteeHeight = config.DefaultParams.
		CRCommitteeStartHeight + config.DefaultParams.CRDutyPeriod

	// < CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod
	assert.False(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod - config.DefaultParams.
				CRVotingPeriod - 1,
		},
	}))

	// CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod
	assert.True(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod - config.DefaultParams.
				CRVotingPeriod,
		},
	}))

	// [CRCommitteeStartHeight + CRDutyPeriod - CRVotingPeriod,
	// CRCommitteeStartHeight + CRDutyPeriod)
	assert.True(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod - config.DefaultParams.
				CRVotingPeriod + 1,
		},
	}))

	// CRCommitteeStartHeight + CRDutyPeriod
	assert.False(t, committee.isInVotingPeriod(&types.Block{
		Header: types.Header{
			Height: config.DefaultParams.CRCommitteeStartHeight +
				config.DefaultParams.CRDutyPeriod,
		},
	}))
}

func generateCandidateSuite() (*StateKeyFrame, []*Candidate) {
	keyFrame := randomStateKeyFrame(24, false)
	candidates := make([]*Candidate, 0, 24)
	for _, v := range keyFrame.ActivityCandidates {
		candidates = append(candidates, v)
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
	return keyFrame, topCandidates
}

func existCode(code []byte, codeArray [][]byte) bool {
	for _, v := range codeArray {
		if bytes.Equal(code, v) {
			return true
		}
	}
	return false
}

func existDID(did common.Uint168, didArray []common.Uint168) bool {
	for _, v := range didArray {
		if v.IsEqual(did) {
			return true
		}
	}
	return false
}
