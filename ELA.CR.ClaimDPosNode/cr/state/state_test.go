package state

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils"

	"github.com/stretchr/testify/assert"
)

func TestState_GetCandidatesRelated(t *testing.T) {
	keyFrame := *randomStateKeyFrame(5, true)
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

	pending := state.GetCandidates(Pending)
	assert.Equal(t, 5, len(pending))

	actives := state.GetCandidates(Active)
	assert.Equal(t, 5, len(actives))

	cancels := state.GetCandidates(Canceled)
	assert.Equal(t, 3, len(cancels))

	returns := state.GetCandidates(Returned)
	assert.Equal(t, 2, len(returns))
}

func TestState_ExistCandidateRelated(t *testing.T) {
	keyFrame := *randomStateKeyFrame(5, true)
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

func TestState_ProcessBlock_PendingUpdateThenCancel(t *testing.T) {
	state := NewState(nil)

	code := randomBytes(34)
	nickname := randomString()
	did := *randomUint168()

	assert.False(t, state.ExistCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))

	// register CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: 1,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, did, nickname),
		},
	}, nil)
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.True(t, state.ExistCandidateByNickname(nickname))
	candidate := state.GetCandidate(code)
	assert.Equal(t, Pending, candidate.state)

	// update pending CR
	nickname2 := randomString()
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: 2,
		},
		Transactions: []*types.Transaction{
			generateUpdateCR(code, did, nickname2),
		},
	}, nil)
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))
	assert.True(t, state.ExistCandidateByNickname(nickname2))
	candidate = state.GetCandidate(code)
	assert.Equal(t, Pending, candidate.state)

	//cancel pending CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: 3,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))
	assert.False(t, state.ExistCandidateByNickname(nickname2))
	candidate = state.GetCandidate(code)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.GetCandidates(Pending)))
	assert.Equal(t, 1, len(state.GetCandidates(Canceled)))
}

func TestState_ProcessBlock_PendingActiveThenCancel(t *testing.T) {
	state := NewState(nil)
	height := uint32(1)

	code := randomBytes(34)
	nickname := randomString()
	did := *randomUint168()

	assert.False(t, state.ExistCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))

	// register CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, did, nickname),
		},
	}, nil)
	height++
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.True(t, state.ExistCandidateByNickname(nickname))
	candidate := state.GetCandidate(code)
	assert.Equal(t, Pending, candidate.state)

	// register CR then after 6 block should be active state
	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	candidate = state.GetCandidate(code)
	assert.Equal(t, Active, candidate.state)

	// update active CR
	nickname2 := randomString()
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateUpdateCR(code, did, nickname2),
		},
	}, nil)
	height++
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))
	assert.True(t, state.ExistCandidateByNickname(nickname2))
	candidate = state.GetCandidate(code)
	assert.Equal(t, Active, candidate.state)

	// cancel active CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	assert.True(t, state.ExistCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))
	assert.False(t, state.ExistCandidateByNickname(nickname2))
	candidate = state.GetCandidate(code)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.GetCandidates(Pending)))
	assert.Equal(t, 1, len(state.GetCandidates(Canceled)))
}

func TestState_ProcessBlock_MixedCRProcessing(t *testing.T) {
	state := State{
		StateKeyFrame: *randomStateKeyFrame(5, true),
		history:       utils.NewHistory(maxHistoryCapacity),
	}
	height := uint32(1)

	assert.Equal(t, 15, len(state.GetAllCandidates()))
	assert.Equal(t, 5, len(state.GetCandidates(Pending)))
	assert.Equal(t, 5, len(state.GetCandidates(Active)))
	assert.Equal(t, 3, len(state.GetCandidates(Canceled)))
	assert.Equal(t, 2, len(state.GetCandidates(Returned)))

	for i := 0; i < 10; i++ {
		code := randomBytes(34)
		nickname := randomString()
		did := *randomUint168()

		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{
				generateRegisterCR(code, did, nickname),
			},
		}, nil)
		height++
	}
	assert.Equal(t, 25, len(state.GetAllCandidates()))
	assert.Equal(t, 5, len(state.GetCandidates(Pending)))
	assert.Equal(t, 15, len(state.GetCandidates(Active)))
	assert.Equal(t, 3, len(state.GetCandidates(Canceled)))
	assert.Equal(t, 2, len(state.GetCandidates(Returned)))

	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	assert.Equal(t, 25, len(state.GetAllCandidates()))
	assert.Equal(t, 0, len(state.GetCandidates(Pending)))
	assert.Equal(t, 20, len(state.GetCandidates(Active)))
	assert.Equal(t, 3, len(state.GetCandidates(Canceled)))
	assert.Equal(t, 2, len(state.GetCandidates(Returned)))
}

func generateRegisterCR(code []byte, did common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.RegisterCR,
		Payload: &payload.CRInfo{
			Code:     code,
			DID:      did,
			NickName: nickname,
		},
	}
}

func generateUpdateCR(code []byte, did common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.UpdateCR,
		Payload: &payload.CRInfo{
			Code:     code,
			DID:      did,
			NickName: nickname,
		},
	}
}

func generateUnregisterCR(code []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.UnregisterCR,
		Payload: &payload.UnregisterCR{
			Code: code,
		},
	}
}
