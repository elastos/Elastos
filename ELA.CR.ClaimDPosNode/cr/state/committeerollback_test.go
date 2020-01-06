// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"bytes"
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

func getCodeByPubKeyStr(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	return redeemScript
}

func getCodeHexStr(publicKey string) string {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	codeHexStr := common.BytesToHexString(redeemScript)
	return codeHexStr
}

func getRegisterCRTx(publicKeyStr, privateKeyStr, nickName string) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)

	code1 := getCodeByPubKeyStr(publicKeyStr1)
	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	did1 := ct1.ToProgramHash()
	hash1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     code1,
		DID:      *did1,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	signBuf := new(bytes.Buffer)
	crInfoPayload.SerializeUnsigned(signBuf, payload.CRInfoVersion)
	rcSig1, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crInfoPayload.Signature = rcSig1
	txn.Payload = crInfoPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *hash1,
		Type:        0,
		Payload:     new(outputpayload.DefaultOutput),
	}}
	return txn
}

func getVoteCRTx(amount common.Fixed64,
	candidateVotes []outputpayload.CandidateVotes) *types.Transaction {
	return &types.Transaction{
		Version: 0x09,
		TxType:  types.TransferAsset,
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{},
				Value:       amount,
				OutputLock:  0,
				ProgramHash: common.Uint168{123},
				Type:        types.OTVote,
				Payload: &outputpayload.VoteOutput{
					Version: outputpayload.VoteProducerAndCRVersion,
					Contents: []outputpayload.VoteContent{
						outputpayload.VoteContent{
							VoteType:       outputpayload.CRC,
							CandidateVotes: candidateVotes,
						},
					},
				},
			},
		},
	}
}

func getDIDByPublicKey(publicKey string) *common.Uint168 {
	code1 := getCodeByPubKeyStr(publicKey)
	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	return ct1.ToProgramHash()
}

func committeeKeyFrameEqual(first *CommitteeKeyFrame, second *CommitteeKeyFrame) bool {
	return keyframeEqual(first.KeyFrame, second.KeyFrame) &&
		stateKeyframeEqual(first.StateKeyFrame, second.StateKeyFrame) &&
		proposalKeyFrameEqual(first.ProposalKeyFrame, second.ProposalKeyFrame)
}

func TestCommittee_RollbackRegisterAndVoteCR(t *testing.T) {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	did1 := getDIDByPublicKey(publicKeyStr1)
	nickName1 := "nickname 1"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	did2 := getDIDByPublicKey(publicKeyStr2)
	nickName2 := "nickname 2"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"
	did3 := getDIDByPublicKey(publicKeyStr3)
	nickName3 := "nickname 3"

	registerCRTxn1 := getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	registerCRTxn2 := getRegisterCRTx(publicKeyStr2, privateKeyStr2, nickName2)
	registerCRTxn3 := getRegisterCRTx(publicKeyStr3, privateKeyStr3, nickName3)

	// new committee
	committee := NewCommittee(&config.DefaultParams)

	// avoid getting UTXOs from database
	currentHeight := config.DefaultParams.CRVotingStartHeight
	committee.recordBalanceHeight = currentHeight - 1

	// register cr
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
	}, nil)
	assert.Equal(t, 3, len(committee.GetCandidates(Pending)))
	assert.Equal(t, 0, len(committee.GetCandidates(Active)))

	// vote cr
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}
	keyFrameA := committee.SnapShort()

	voteCRTx := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did1.Bytes(), 3},
		{did2.Bytes(), 2},
		{did3.Bytes(), 1}})
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx,
		},
	}, nil)
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)
	keyFrameB := committee.SnapShort()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, common.Fixed64(0), committee.GetCandidate(*did1).votes)
	keyFrameC := committee.SnapShort()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteCRTx}}, nil)
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)
	keyFrameD := committee.SnapShort()

	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameA, keyFrameC))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameA, keyFrameB))
	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameB, keyFrameD))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameB, keyFrameC))
}

func TestCommittee_RollbackEndVotingPeriod(t *testing.T) {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	did1 := getDIDByPublicKey(publicKeyStr1)
	nickName1 := "nickname 1"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	did2 := getDIDByPublicKey(publicKeyStr2)
	nickName2 := "nickname 2"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"
	did3 := getDIDByPublicKey(publicKeyStr3)
	nickName3 := "nickname 3"

	registerCRTxn1 := getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	registerCRTxn2 := getRegisterCRTx(publicKeyStr2, privateKeyStr2, nickName2)
	registerCRTxn3 := getRegisterCRTx(publicKeyStr3, privateKeyStr3, nickName3)

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2

	// new committee
	committee := NewCommittee(cfg)

	// avoid getting UTXOs from database
	currentHeight := config.DefaultParams.CRVotingStartHeight
	committee.recordBalanceHeight = currentHeight - 1

	// register cr
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
	}, nil)
	assert.Equal(t, 3, len(committee.GetCandidates(Pending)))
	assert.Equal(t, 0, len(committee.GetCandidates(Active)))

	// vote cr
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}

	voteCRTx := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did1.Bytes(), 3},
		{did2.Bytes(), 2},
		{did3.Bytes(), 1}})

	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx,
		},
	}, nil)

	currentHeight = cfg.CRCommitteeStartHeight - 1
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 0, len(committee.GetAllMembers()))
	assert.Equal(t, 3, len(committee.GetAllCandidates()))

	// process
	keyFrameA := committee.SnapShort()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameB := committee.SnapShort()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 0, len(committee.GetAllMembers()))
	assert.Equal(t, 3, len(committee.GetAllCandidates()))
	keyFrameC := committee.SnapShort()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameD := committee.SnapShort()

	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameA, keyFrameC))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameA, keyFrameB))
	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameB, keyFrameD))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameB, keyFrameC))
}

func TestCommittee_RollbackContinueVotingPeriod(t *testing.T) {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	did1 := getDIDByPublicKey(publicKeyStr1)
	nickName1 := "nickname 1"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	did2 := getDIDByPublicKey(publicKeyStr2)
	nickName2 := "nickname 2"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"
	did3 := getDIDByPublicKey(publicKeyStr3)
	nickName3 := "nickname 3"

	registerCRTxn1 := getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	registerCRTxn2 := getRegisterCRTx(publicKeyStr2, privateKeyStr2, nickName2)
	registerCRTxn3 := getRegisterCRTx(publicKeyStr3, privateKeyStr3, nickName3)

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:4]
	cfg.CRMemberCount = 4

	// new committee
	committee := NewCommittee(cfg)

	// avoid getting UTXOs from database
	currentHeight := config.DefaultParams.CRVotingStartHeight
	committee.recordBalanceHeight = currentHeight - 1

	// register cr
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
	}, nil)
	assert.Equal(t, 3, len(committee.GetCandidates(Pending)))
	assert.Equal(t, 0, len(committee.GetCandidates(Active)))

	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
	}, nil)

	// vote cr
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}

	voteCRTx := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did1.Bytes(), 3},
		{did2.Bytes(), 2},
		{did3.Bytes(), 1}})

	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx,
		},
	}, nil)

	currentHeight = cfg.CRCommitteeStartHeight - 1
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameA := committee.SnapShort()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameB := committee.SnapShort()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	keyFrameC := committee.SnapShort()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameD := committee.SnapShort()

	publicKeyStr4 := "027209c3a6bcb95e9ef766c81136bcd6f2338eee7f9caebf694825e411320bab12"
	privateKeyStr4 := "b3b1c16abd786c4994af9ee8c79d25457f66509731f74d6a9a9673ca872fa8fa"
	did4 := getDIDByPublicKey(publicKeyStr4)
	nickName4 := "nickname 4"
	registerCRTxn4 := getRegisterCRTx(publicKeyStr4, privateKeyStr4, nickName4)

	// register
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn4,
		},
	}, nil)

	// vote cr
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}
	voteCRTx2 := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did4.Bytes(), 4}})

	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx2,
		},
	}, nil)

	// set current height to one block before ending voting period
	currentHeight = cfg.CRCommitteeStartHeight - 1 + cfg.CRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameA2 := committee.SnapShort()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 4, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameB2 := committee.SnapShort()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	keyFrameC2 := committee.SnapShort()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 4, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameD2 := committee.SnapShort()

	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameA, keyFrameC))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameA, keyFrameB))
	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameB, keyFrameD))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameB, keyFrameC))

	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameA2, keyFrameC2))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameA2, keyFrameB2))
	assert.Equal(t, true, committeeKeyFrameEqual(keyFrameB2, keyFrameD2))
	assert.Equal(t, false, committeeKeyFrameEqual(keyFrameB2, keyFrameC2))
}
