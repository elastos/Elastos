// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
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
	for k, v := range keyFrame.Candidates {
		v2 := state.getCandidate(k)
		assert.True(t, candidateEqual(v, v2))

		v3 := state.getCandidate(v.info.DID)
		assert.True(t, candidateEqual(v, v3))
	}

	// get candidates
	candidates := state.getAllCandidates()
	assert.Equal(t, 15, len(candidates))

	pending := state.getCandidates(Pending)
	assert.Equal(t, 5, len(pending))

	actives := state.getCandidates(Active)
	assert.Equal(t, 5, len(actives))

	cancels := state.getCandidates(Canceled)
	assert.Equal(t, 3, len(cancels))

	returns := state.getCandidates(Returned)
	assert.Equal(t, 2, len(returns))
}

func TestState_ExistCandidateRelated(t *testing.T) {
	keyFrame := *randomStateKeyFrame(5, true)
	state := State{
		StateKeyFrame: keyFrame,
	}

	assert.False(t, state.existCandidate(make([]byte, 34)))
	assert.False(t, state.ExistCandidateByDID(common.Uint168{}))
	assert.False(t, state.existCandidateByNickname(""))

	for _, v := range keyFrame.Candidates {
		assert.True(t, state.existCandidate(v.info.Code))
		assert.True(t, state.ExistCandidateByDID(v.info.DID))
		assert.True(t, state.existCandidateByNickname(v.info.NickName))
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

	assert.False(t, state.existCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))

	registerFuncs(state)

	// register CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: 1,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, did, nickname),
		},
	}, nil, 0)
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.True(t, state.existCandidateByNickname(nickname))
	candidate := state.getCandidate(did)
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
	}, nil, 0)
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))
	assert.True(t, state.existCandidateByNickname(nickname2))
	candidate = state.getCandidate(did)
	assert.Equal(t, Pending, candidate.state)

	//cancel pending CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: 3,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil, 0)
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))
	assert.False(t, state.existCandidateByNickname(nickname2))
	candidate = state.getCandidate(did)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.getCandidates(Pending)))
	assert.Equal(t, 1, len(state.getCandidates(Canceled)))
}

func TestState_ProcessBlock_PendingActiveThenCancel(t *testing.T) {
	state := NewState(nil)
	height := uint32(1)
	nickname := randomString()
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getCode(publicKeyStr1)
	did := *getDid(code)

	assert.False(t, state.existCandidate(code))
	assert.False(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))

	registerFuncs(state)

	// register CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, did, nickname),
		},
	}, nil, 0)
	height++
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.True(t, state.existCandidateByNickname(nickname))
	candidate := state.getCandidate(did)
	assert.Equal(t, Pending, candidate.state)

	// register CR then after 6 block should be active state
	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil, 0)
		height++
	}
	candidate = state.getCandidate(did)
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
	}, nil, 0)
	height++
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))
	assert.True(t, state.existCandidateByNickname(nickname2))
	candidate = state.getCandidate(did)
	assert.Equal(t, Active, candidate.state)

	// cancel active CR
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil, 0)
	assert.True(t, state.existCandidate(code))
	assert.True(t, state.ExistCandidateByDID(did))
	assert.False(t, state.existCandidateByNickname(nickname))
	assert.False(t, state.existCandidateByNickname(nickname2))
	candidate = state.getCandidate(did)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(state.getCandidates(Pending)))
	assert.Equal(t, 1, len(state.getCandidates(Canceled)))
}

func TestState_ProcessBlock_MixedCRProcessing(t *testing.T) {
	state := State{
		StateKeyFrame: *randomStateKeyFrame(5, true),
		history:       utils.NewHistory(maxHistoryCapacity),
	}
	registerFuncs(&state)
	height := uint32(1)

	assert.Equal(t, 15, len(state.getAllCandidates()))
	assert.Equal(t, 5, len(state.getCandidates(Pending)))
	assert.Equal(t, 5, len(state.getCandidates(Active)))
	assert.Equal(t, 3, len(state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(state.getCandidates(Returned)))

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
		}, nil, 0)
		height++
	}
	assert.Equal(t, 25, len(state.getAllCandidates()))
	assert.Equal(t, 5, len(state.getCandidates(Pending)))
	assert.Equal(t, 15, len(state.getCandidates(Active)))
	assert.Equal(t, 3, len(state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(state.getCandidates(Returned)))

	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil, 0)
		height++
	}
	assert.Equal(t, 25, len(state.getAllCandidates()))
	assert.Equal(t, 0, len(state.getCandidates(Pending)))
	assert.Equal(t, 20, len(state.getCandidates(Active)))
	assert.Equal(t, 3, len(state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(state.getCandidates(Returned)))
}

func TestState_ProcessBlock_VotingAndCancel(t *testing.T) {
	keyframe := randomStateKeyFrame(5, true)
	state := NewState(&config.DefaultParams)
	state.StateKeyFrame = *keyframe
	state.history = utils.NewHistory(maxHistoryCapacity)
	height := uint32(1)

	activeDIDs := make([][]byte, 0, 5)
	for k, v := range keyframe.Candidates {
		v.votes = 0
		activeDIDs = append(activeDIDs, k.Bytes())
	}

	registerFuncs(state)
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
	}, nil, 0)
	height++

	for i, v := range activeDIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := state.getCandidate(*did)
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
	}, nil, 0)

	for _, v := range activeDIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := state.getCandidate(*did)
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
				Value:       common.Fixed64(6000 * 1e8),
			},
		},
	}
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{registerCRTx},
	}, nil, 0)
	height++
	candidate := state.getCandidate(did)
	assert.Equal(t, common.Fixed64(5000*1e8),
		state.getDepositAmount(candidate.info.DID))
	assert.Equal(t, common.Fixed64(6000*1e8),
		state.getTotalAmount(candidate.info.DID))

	// deposit though normal tx
	tranferTx := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				ProgramHash: *depositCont.ToProgramHash(),
				Value:       common.Fixed64(1000 * 1e8),
			},
		},
	}
	state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{tranferTx},
	}, nil, 0)
	height++
	assert.Equal(t, common.Fixed64(5000*1e8),
		state.getDepositAmount(candidate.info.DID))
	assert.Equal(t, common.Fixed64(7000*1e8),
		state.getTotalAmount(candidate.info.DID))

	// cancel candidate
	for i := 0; i < 4; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil, 0)
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
	}, nil, 0)
	height++
	for i := 0; i < 5; i++ {
		state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil, 0)
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
	}, nil, 0)
	state.history.Commit(height)
	assert.Equal(t, common.Fixed64(0), state.getDepositAmount(candidate.info.DID))
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
	state.registerFunctions(&FunctionsConfig{
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
