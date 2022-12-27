// Copyright (c) 2017-2020 The Elastos Foundation
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

		v3 := state.getCandidate(v.info.CID)
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
	assert.False(t, state.ExistCandidateByCID(common.Uint168{}))
	assert.False(t, state.existCandidateByNickname(""))

	for _, v := range keyFrame.Candidates {
		assert.True(t, state.existCandidate(v.info.Code))
		assert.True(t, state.ExistCandidateByCID(v.info.CID))
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
	cfg := &config.DefaultParams
	cfg.CRVotingStartHeight = 0
	currentHeight := uint32(1)
	committee := NewCommittee(cfg)
	committee.RegisterFuncitons(&CommitteeFuncsConfig{
		GetHeight: func() uint32 {
			return currentHeight
		},
		GetUTXO: func(programHash *common.Uint168) ([]*types.UTXO, error) {
			return []*types.UTXO{}, nil
		},
	})
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getCode(publicKeyStr1)
	cid := *getCID(code)
	nickname := randomString()

	assert.False(t, committee.state.existCandidate(code))
	assert.False(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))

	registerFuncs(committee.state)

	// register CR
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, cid, nickname),
		},
	}, nil)
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.True(t, committee.state.existCandidateByNickname(nickname))
	candidate := committee.state.getCandidate(cid)
	assert.Equal(t, Pending, candidate.state)

	// update pending CR
	nickname2 := randomString()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateUpdateCR(code, cid, nickname2),
		},
	}, nil)
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))
	assert.True(t, committee.state.existCandidateByNickname(nickname2))
	candidate = committee.state.getCandidate(cid)
	assert.Equal(t, Pending, candidate.state)

	//cancel pending CR
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))
	assert.False(t, committee.state.existCandidateByNickname(nickname2))
	candidate = committee.state.getCandidate(cid)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(committee.state.getCandidates(Pending)))
	assert.Equal(t, 1, len(committee.state.getCandidates(Canceled)))
}

func TestState_ProcessBlock_PendingActiveThenCancel(t *testing.T) {
	cfg := &config.DefaultParams
	cfg.CRVotingStartHeight = 0
	currentHeight := uint32(1)
	committee := NewCommittee(cfg)
	committee.RegisterFuncitons(&CommitteeFuncsConfig{
		GetHeight: func() uint32 {
			return currentHeight
		},
		GetUTXO: func(programHash *common.Uint168) ([]*types.UTXO, error) {
			return []*types.UTXO{}, nil
		},
	})
	nickname := randomString()
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getCode(publicKeyStr1)
	cid := *getCID(code)

	assert.False(t, committee.state.existCandidate(code))
	assert.False(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))

	registerFuncs(committee.state)

	// register CR
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateRegisterCR(code, cid, nickname),
		},
	}, nil)
	currentHeight++
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.True(t, committee.state.existCandidateByNickname(nickname))
	candidate := committee.state.getCandidate(cid)
	assert.Equal(t, Pending, candidate.state)

	// register CR then after 6 block should be active state
	for i := 0; i < 5; i++ {
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		currentHeight++
	}
	candidate = committee.state.getCandidate(cid)
	assert.Equal(t, Active, candidate.state)

	// update active CR
	nickname2 := randomString()
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateUpdateCR(code, cid, nickname2),
		},
	}, nil)
	currentHeight++
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))
	assert.True(t, committee.state.existCandidateByNickname(nickname2))
	candidate = committee.state.getCandidate(cid)
	assert.Equal(t, Active, candidate.state)

	// cancel active CR
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	assert.True(t, committee.state.existCandidate(code))
	assert.True(t, committee.state.ExistCandidateByCID(cid))
	assert.False(t, committee.state.existCandidateByNickname(nickname))
	assert.False(t, committee.state.existCandidateByNickname(nickname2))
	candidate = committee.state.getCandidate(cid)
	assert.Equal(t, Canceled, candidate.state)
	assert.Equal(t, 0, len(committee.state.getCandidates(Pending)))
	assert.Equal(t, 1, len(committee.state.getCandidates(Canceled)))
}

func TestState_ProcessBlock_MixedCRProcessing(t *testing.T) {
	cfg := &config.DefaultParams
	cfg.CRVotingStartHeight = 0
	currentHeight := uint32(1)
	committee := NewCommittee(cfg)
	committee.state.StateKeyFrame = *randomStateKeyFrame(5, true)
	committee.RegisterFuncitons(&CommitteeFuncsConfig{
		GetHeight: func() uint32 {
			return currentHeight
		},
		GetUTXO: func(programHash *common.Uint168) ([]*types.UTXO, error) {
			return []*types.UTXO{}, nil
		},
	})
	registerFuncs(committee.state)

	assert.Equal(t, 15, len(committee.state.getAllCandidates()))
	assert.Equal(t, 5, len(committee.state.getCandidates(Pending)))
	assert.Equal(t, 5, len(committee.state.getCandidates(Active)))
	assert.Equal(t, 3, len(committee.state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(committee.state.getCandidates(Returned)))

	for i := 0; i < 10; i++ {
		code := randomBytes(34)
		nickname := randomString()
		cid := *randomUint168()

		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
			Transactions: []*types.Transaction{
				generateRegisterCR(code, cid, nickname),
			},
		}, nil)
		currentHeight++
	}
	assert.Equal(t, 25, len(committee.state.getAllCandidates()))
	assert.Equal(t, 5, len(committee.state.getCandidates(Pending)))
	assert.Equal(t, 15, len(committee.state.getCandidates(Active)))
	assert.Equal(t, 3, len(committee.state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(committee.state.getCandidates(Returned)))

	for i := 0; i < 5; i++ {
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		currentHeight++
	}
	assert.Equal(t, 25, len(committee.state.getAllCandidates()))
	assert.Equal(t, 0, len(committee.state.getCandidates(Pending)))
	assert.Equal(t, 20, len(committee.state.getCandidates(Active)))
	assert.Equal(t, 3, len(committee.state.getCandidates(Canceled)))
	assert.Equal(t, 2, len(committee.state.getCandidates(Returned)))
}

func TestState_ProcessBlock_VotingAndCancel(t *testing.T) {
	cfg := &config.DefaultParams
	cfg.CRVotingStartHeight = 0
	currentHeight := uint32(1)
	keyframe := randomStateKeyFrame(5, true)
	committee := NewCommittee(cfg)
	committee.state.StateKeyFrame = *keyframe
	committee.RegisterFuncitons(&CommitteeFuncsConfig{
		GetHeight: func() uint32 {
			return currentHeight
		},
		GetUTXO: func(programHash *common.Uint168) ([]*types.UTXO, error) {
			return []*types.UTXO{}, nil
		},
	})

	activeCIDs := make([][]byte, 0, 5)
	for k, v := range keyframe.Candidates {
		v.votes = 0
		activeCIDs = append(activeCIDs, k.Bytes())
	}

	registerFuncs(committee.state)
	references := make(map[*types.Input]types.Output)
	committee.state.getTxReference = func(tx *types.Transaction) (
		map[*types.Input]types.Output, error) {
		return references, nil
	}

	// vote for the active candidates
	voteTx := mockNewVoteTx(activeCIDs)
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{voteTx},
	}, nil)
	currentHeight++

	for i, v := range activeCIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := committee.state.getCandidate(*did)
		assert.Equal(t, common.Fixed64((i+1)*10), candidate.votes)
	}

	input := &types.Input{
		Previous: *types.NewOutPoint(voteTx.Hash(), uint16(0)),
	}
	references[input] = *voteTx.Outputs[0]

	// cancel votes the active candidates
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			{
				Inputs: []*types.Input{input},
			},
		},
	}, nil)

	for _, v := range activeCIDs {
		did, _ := common.Uint168FromBytes(v)
		candidate := committee.state.getCandidate(*did)
		assert.Equal(t, common.Fixed64(0), candidate.votes)
	}
}

func TestState_ProcessBlock_DepositAndReturnDeposit(t *testing.T) {
	cfg := &config.DefaultParams
	cfg.CRVotingStartHeight = 0
	currentHeight := uint32(1)
	committee := NewCommittee(cfg)
	committee.RegisterFuncitons(&CommitteeFuncsConfig{
		GetHeight: func() uint32 {
			return currentHeight
		},
		GetUTXO: func(programHash *common.Uint168) ([]*types.UTXO, error) {
			return []*types.UTXO{}, nil
		},
	})
	registerFuncs(committee.state)

	_, pk, _ := crypto.GenerateKeyPair()
	cont, _ := contract.CreateStandardContract(pk)
	code := cont.Code
	cid := *getCID(code)

	depositCont, _ := contract.CreateDepositContractByPubKey(pk)

	// register CR
	registerCRTx := &types.Transaction{
		TxType: types.RegisterCR,
		Payload: &payload.CRInfo{
			Code:     code,
			CID:      cid,
			NickName: randomString(),
		},
		Outputs: []*types.Output{
			{
				ProgramHash: *depositCont.ToProgramHash(),
				Value:       common.Fixed64(6000 * 1e8),
			},
		},
	}
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{registerCRTx},
	}, nil)
	currentHeight++
	candidate := committee.state.getCandidate(cid)
	assert.Equal(t, common.Fixed64(5000*1e8),
		committee.state.getDepositAmount(candidate.info.CID))
	assert.Equal(t, common.Fixed64(6000*1e8),
		committee.state.getTotalAmount(candidate.info.CID))

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
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{tranferTx},
	}, nil)
	currentHeight++
	assert.Equal(t, common.Fixed64(5000*1e8),
		committee.state.getDepositAmount(candidate.info.CID))
	assert.Equal(t, common.Fixed64(7000*1e8),
		committee.state.getTotalAmount(candidate.info.CID))

	// cancel candidate
	for i := 0; i < 4; i++ {
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		currentHeight++
	}
	assert.Equal(t, Active, candidate.state)
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			generateUnregisterCR(code),
		},
	}, nil)
	cancelHeight := currentHeight
	currentHeight++
	for i := 0; i < 5; i++ {
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		currentHeight++
	}
	assert.Equal(t, Canceled, candidate.state)

	// reached the height to return deposit amount.
	currentHeight = cancelHeight + committee.params.CRDepositLockupBlocks
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{},
	}, nil)

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
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{rdTx},
	}, nil)
	committee.state.history.Commit(currentHeight)
	assert.Equal(t, common.Fixed64(0),
		committee.state.getDepositAmount(candidate.info.CID))
}

func mockNewVoteTx(cids [][]byte) *types.Transaction {
	candidateVotes := make([]outputpayload.CandidateVotes, 0, len(cids))
	for i, cid := range cids {
		//code := getCode(common.BytesToHexString(pk))
		candidateVotes = append(candidateVotes,
			outputpayload.CandidateVotes{
				Candidate: cid,
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

func generateRegisterCR(code []byte, cid common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.RegisterCR,
		Payload: &payload.CRInfo{
			Code:     code,
			CID:      cid,
			NickName: nickname,
		},
	}
}

func generateUpdateCR(code []byte, cid common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.UpdateCR,
		Payload: &payload.CRInfo{
			Code:     code,
			CID:      cid,
			NickName: nickname,
		},
	}
}

func generateUnregisterCR(code []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.UnregisterCR,
		Payload: &payload.UnregisterCR{
			CID: *getCID(code),
		},
	}
}

func getCID(code []byte) *common.Uint168 {
	ct1, _ := contract.CreateCRIDContractByCode(code)
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
		GetHistoryMember: func(code []byte) []*CRMember { return nil },
		GetTxReference: func(tx *types.Transaction) (
			map[*types.Input]types.Output, error) {
			return make(map[*types.Input]types.Output), nil
		}})
}
