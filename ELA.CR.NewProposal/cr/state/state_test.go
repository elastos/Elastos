package state

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/stretchr/testify/assert"
)

func TestState_GetCandidatesRelated(t *testing.T) {
	keyFrame := *randomStateKeyFrame(5)
	state := State{
		StateKeyFrame: keyFrame,
	}

	// get single candidate
	for k, v := range keyFrame.PendingCandidates {
		v2 := state.GetCandidateByDID(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.Code)
		assert.True(t, candidateEqual(v, v3))
	}
	for k, v := range keyFrame.ActivityCandidates {
		v2 := state.GetCandidateByDID(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.Code)
		assert.True(t, candidateEqual(v, v3))
	}
	for k, v := range keyFrame.CanceledCandidates {
		v2 := state.GetCandidateByDID(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.Code)
		assert.True(t, candidateEqual(v, v3))
	}

	// get candidates
	candidates := state.GetAllCandidates()
	assert.Equal(t, 15, len(candidates))

	pendings := state.GetCandidates(Pending)
	assert.Equal(t, 5, len(pendings))

	actives := state.GetCandidates(Active)
	assert.Equal(t, 5, len(actives))

	cancels := state.GetCandidates(Canceled)
	assert.Equal(t, 3, len(cancels))

	returns := state.GetCandidates(Returned)
	assert.Equal(t, 2, len(returns))
}

func TestState_ExistCandidateRelated(t *testing.T) {
	keyFrame := *randomStateKeyFrame(5)
	state := State{
		StateKeyFrame: keyFrame,
	}

	assert.False(t, state.ExistCandidate(make([]byte, 34)))
	assert.False(t, state.ExistCandidateByDID(common.Uint168{}))
	assert.False(t, state.ExistCandidateByNickname(""))

	for _, v := range keyFrame.PendingCandidates {
		assert.True(t, state.ExistCandidate(v.info.Code))
		assert.True(t, state.ExistCandidateByDID(v.info.DID))
		assert.True(t, state.ExistCandidateByNickname(v.info.NickName))
	}

	for _, v := range keyFrame.ActivityCandidates {
		assert.True(t, state.ExistCandidate(v.info.Code))
		assert.True(t, state.ExistCandidateByDID(v.info.DID))
		assert.True(t, state.ExistCandidateByNickname(v.info.NickName))
	}

	for _, v := range keyFrame.CanceledCandidates {
		assert.True(t, state.ExistCandidate(v.info.Code))
		assert.True(t, state.ExistCandidateByDID(v.info.DID))
		assert.True(t, state.ExistCandidateByNickname(v.info.NickName))
	}
}
