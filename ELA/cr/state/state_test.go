// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"github.com/elastos/Elastos.ELA/common/config"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
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
		v2 := state.GetCandidate(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.DID)
		assert.True(t, candidateEqual(v, v3))
	}
	for k, v := range keyFrame.ActivityCandidates {
		v2 := state.GetCandidate(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.DID)
		assert.True(t, candidateEqual(v, v3))
	}
	for k, v := range keyFrame.CanceledCandidates {
		v2 := state.GetCandidate(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.GetCandidate(v.info.DID)
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

func getCode(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	return redeemScript
}

func TestState_ProcessBlock_PendingUpdateThenCancel(t *testing.T) {
	state := NewState(nil)
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getCode(publicKeyStr1)
	did := *getDid(code)
	nickname := randomString()

	assert.False(t, state.ExistCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))

	registerFuncs(state)

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
	candidate := state.GetCandidate(did)
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
	candidate = state.GetCandidate(did)
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
	candidate = state.GetCandidate(did)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.GetCandidates(Pending)))
	assert.Equal(t, 1, len(state.GetCandidates(Canceled)))
}

func TestState_ProcessBlock_PendingActiveThenCancel(t *testing.T) {
	state := NewState(nil)
	height := uint32(1)
	nickname := randomString()
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getCode(publicKeyStr1)
	did := *getDid(code)

	assert.False(t, state.ExistCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.ExistCandidateByNickname(nickname))

	registerFuncs(state)

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
	candidate := state.GetCandidate(did)
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
	candidate = state.GetCandidate(did)
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
	candidate = state.GetCandidate(did)
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
	candidate = state.GetCandidate(did)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.GetCandidates(Pending)))
	assert.Equal(t, 1, len(state.GetCandidates(Canceled)))
}

func TestState_ProcessBlock_MixedCRProcessing(t *testing.T) {
	state := State{
		StateKeyFrame: *randomStateKeyFrame(5, true),
		history:       utils.NewHistory(maxHistoryCapacity),
	}
	registerFuncs(&state)
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

func TestState_ProcessBlock_VotingAndCancel(t *testing.T) {
	keyframe := randomStateKeyFrame(5, true)
	state := State{
		params:        &config.DefaultParams,
		StateKeyFrame: *keyframe,
		history:       utils.NewHistory(maxHistoryCapacity),
	}
	height := uint32(1)

	activeDIDs := make([][]byte, 0, 5)
	for k, v := range keyframe.ActivityCandidates {
		v.votes = 0
		activeDIDs = append(activeDIDs, k.Bytes())
	}

	registerFuncs(&state)
	references := make(map[*types.Input]*types.Output)
	state.getTxReference = func(tx *types.Transaction) (
		map[*types.Input]*types.Output, error) {
		return references, nil
	}

	// vote for the active candidates
	voteTx := mockNewVoteTx(activeDIDs)
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{voteTx},
	}, nil)
	height++

	for i, v := range activeDIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := state.GetCandidate(*did)
		assert.Equal(t, common.Fixed64((i+1)*10), candidate.votes)
	}

	input := &types.Input{
		Previous: *types.NewOutPoint(voteTx.Hash(), uint16(0)),
	}
	references[input] = voteTx.Outputs[0]

	// cancel votes the active candidates
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			{
				Inputs: []*types.Input{input},
			},
		},
	}, nil)

	for _, v := range activeDIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := state.GetCandidate(*did)
		assert.Equal(t, common.Fixed64(0), candidate.votes)
	}
}

func TestState_ProcessBlock_DepositAndReturnDeposit(t *testing.T) {
	state := NewState(&config.DefaultParams)
	registerFuncs(state)
	height := uint32(1)

	_, pk, _ := crypto.GenerateKeyPair()
	cont, _ := contract.CreateStandardContract(pk)
	code := cont.Code
	did := *getDid(code)

	depositCont, _ := contract.CreateDepositContractByPubKey(pk)

	// register CR
	registerCRTx := &types.Transaction{
		TxType: types.RegisterCR,
		Payload: &payload.CRInfo{
			Code:     code,
			DID:      *getDid(code),
			NickName: randomString(),
		},
		Outputs: []*types.Output{
			{
				ProgramHash: *depositCont.ToProgramHash(),
				Value:       common.Fixed64(100),
			},
		},
	}
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{registerCRTx},
	}, nil)
	height++
	candidate := state.GetCandidate(did)
	assert.Equal(t, common.Fixed64(100), candidate.depositAmount)

	// deposit though normal tx
	tranferTx := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				ProgramHash: *depositCont.ToProgramHash(),
				Value:       common.Fixed64(200),
			},
		},
	}
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{tranferTx},
	}, nil)
	height++
	assert.Equal(t, common.Fixed64(300), candidate.depositAmount)

	// cancel candidate
	for i := 0; i < 4; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	assert.Equal(t, Active, candidate.state)
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	height++
	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	assert.Equal(t, Canceled, candidate.state)

	// return deposit
	rdTx := generateReturnCRDeposit(code)
	rdTx.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  registerCRTx.Hash(),
				Index: 0,
			},
		},
		{
			Previous: types.OutPoint{
				TxID:  tranferTx.Hash(),
				Index: 0,
			},
		},
	}
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{rdTx},
	}, nil)
	state.history.Commit(height)
	assert.Equal(t, common.Fixed64(0), candidate.depositAmount)
}

func mockNewVoteTx(dids [][]byte) *types.Transaction {
	candidateVotes := make([]outputpayload.CandidateVotes, 0, len(dids))
	for i, did := range dids {
		//code := getCode(common.BytesToHexString(pk))
		candidateVotes = append(candidateVotes,
			outputpayload.CandidateVotes{
				Candidate: did,
				Votes:     common.Fixed64((i + 1) * 10)})
	}
	output := &types.Output{
		Value: 100,
		Type:  types.OTVote,
		Payload: &outputpayload.VoteOutput{
			Version: outputpayload.VoteProducerAndCRVersion,
			Contents: []outputpayload.VoteContent{
				{outputpayload.CRC, candidateVotes},
			},
		},
	}

	return &types.Transaction{
		Version: types.TxVersion09,
		TxType:  types.TransferAsset,
		Outputs: []*types.Output{output},
	}
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
			DID: *getDid(code),
		},
	}
}

func getDid(code []byte) *common.Uint168 {
	ct1, _ := contract.CreateCRDIDContractByCode(code)
	return ct1.ToProgramHash()
}

func generateReturnCRDeposit(code []byte) *types.Transaction {
	return &types.Transaction{
		TxType:  types.ReturnCRDepositCoin,
		Payload: &payload.ReturnDepositCoin{},
		Programs: []*program.Program{
			&program.Program{
				Code: code,
			},
		},
	}
}

func registerFuncs(state *State) {
	state.RegisterFunctions(&FunctionsConfig{
		GetHistoryMember: func(code []byte) *CRMember { return nil },
		ProcessCRCRelatedAmount: func(tx *types.Transaction, height uint32,
			history *utils.History, foundationInputsAmounts map[string]common.Fixed64,
			committeeInputsAmounts map[string]common.Fixed64) {
		},
		GetTxReference: func(tx *types.Transaction) (
			map[*types.Input]*types.Output, error) {
			return make(map[*types.Input]*types.Output), nil
		}})
}
