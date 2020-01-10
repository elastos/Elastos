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

func getDIDByPublicKey(publicKey string) *common.Uint168 {
	code1 := getCodeByPubKeyStr(publicKey)
	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	return ct1.ToProgramHash()
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

func getCRCProposalTx(elaAddress string, publicKeyStr, privateKeyStr,
	crPublicKeyStr, crPrivateKeyStr string) *types.Transaction {
	publicKey1, _ := common.HexStringToBytes(publicKeyStr)
	privateKey1, _ := common.HexStringToBytes(privateKeyStr)

	privateKey2, _ := common.HexStringToBytes(crPrivateKeyStr)
	code2 := getCodeByPubKeyStr(crPublicKeyStr)

	recipient, _ := common.Uint168FromAddress(elaAddress)

	draftData := randomBytes(10)
	opinionHash := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposal
	txn.Version = types.TxVersion09
	crcProposalPayload := &payload.CRCProposal{
		ProposalType:     payload.Normal,
		SponsorPublicKey: publicKey1,
		CRSponsorDID:     *getDid(code2),
		DraftHash:        common.Hash(draftData),
		CROpinionHash:    common.Hash(opinionHash),
		Budgets:          createBudgets(3),
		Recipient:        *recipient,
	}

	signBuf := new(bytes.Buffer)
	crcProposalPayload.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalPayload.Sign = sig

	common.WriteVarBytes(signBuf, sig)
	crcProposalPayload.CRSponsorDID.Serialize(signBuf)
	crSig, _ := crypto.Sign(privateKey2, signBuf.Bytes())
	crcProposalPayload.CRSign = crSig

	txn.Payload = crcProposalPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func getCRCProposalReviewTx(proposalHash common.Uint256, vote payload.VoteResult,
	crPublicKeyStr, crPrivateKeyStr string) *types.Transaction {

	privateKey1, _ := common.HexStringToBytes(crPrivateKeyStr)
	code := getCodeByPubKeyStr(crPublicKeyStr)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalReview
	txn.Version = types.TxVersion09
	crcProposalReviewPayload := &payload.CRCProposalReview{
		ProposalHash: proposalHash,
		VoteResult:   vote,
		DID:          *getDid(code),
	}

	signBuf := new(bytes.Buffer)
	crcProposalReviewPayload.SerializeUnsigned(signBuf, payload.CRCProposalReviewVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalReviewPayload.Sign = sig

	txn.Payload = crcProposalReviewPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(crPublicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func getCRCProposalTrackingTx(
	trackingType payload.CRCProposalTrackingType,
	proposalHash common.Uint256, stage uint8,
	leaderPublicKeyStr, leaderPrivateKeyStr,
	newLeaderPublicKeyStr, newLeaderPrivateKeyStr,
	sgPrivateKeyStr string) *types.Transaction {

	leaderPublicKey, _ := common.HexStringToBytes(leaderPublicKeyStr)
	leaderPrivateKey, _ := common.HexStringToBytes(leaderPrivateKeyStr)

	newLeaderPublicKey, _ := common.HexStringToBytes(newLeaderPublicKeyStr)
	newLeaderPrivateKey, _ := common.HexStringToBytes(newLeaderPrivateKeyStr)

	sgPrivateKey, _ := common.HexStringToBytes(sgPrivateKeyStr)

	documentData := randomBytes(10)
	opinionHash := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalTracking
	txn.Version = types.TxVersion09
	cPayload := &payload.CRCProposalTracking{
		ProposalTrackingType: trackingType,
		ProposalHash:         proposalHash,
		Stage:                stage,
		DocumentHash:         common.Hash(documentData),
		LeaderPubKey:         leaderPublicKey,
		NewLeaderPubKey:      newLeaderPublicKey,
		SecretaryOpinionHash: common.Hash(opinionHash),
	}

	signBuf := new(bytes.Buffer)
	cPayload.SerializeUnsigned(signBuf, payload.CRCProposalTrackingVersion)
	sig, _ := crypto.Sign(leaderPrivateKey, signBuf.Bytes())
	cPayload.LeaderSign = sig

	if newLeaderPublicKeyStr != "" && newLeaderPrivateKeyStr != "" {
		common.WriteVarBytes(signBuf, sig)
		crSig, _ := crypto.Sign(newLeaderPrivateKey, signBuf.Bytes())
		cPayload.NewLeaderSign = crSig
		sig = crSig
	}

	common.WriteVarBytes(signBuf, sig)
	crSig, _ := crypto.Sign(sgPrivateKey, signBuf.Bytes())
	cPayload.SecretaryGeneralSign = crSig

	txn.Payload = cPayload
	return txn
}

func getCRCProposalWithdrawTx(proposalHash common.Uint256,
	sponsorPublicKeyStr, sponsorPrivateKeyStr string, fee common.Fixed64,
	inputs []*types.Input, outputs []*types.Output) *types.Transaction {

	sponsorPublicKey, _ := common.HexStringToBytes(sponsorPublicKeyStr)
	sponsorPrivateKey, _ := common.HexStringToBytes(sponsorPrivateKeyStr)

	crcProposalWithdraw := &payload.CRCProposalWithdraw{
		ProposalHash:     proposalHash,
		SponsorPublicKey: sponsorPublicKey,
		Fee:              fee,
	}

	signBuf := new(bytes.Buffer)
	crcProposalWithdraw.SerializeUnsigned(signBuf, payload.CRCProposalWithdrawVersion)
	signature, _ := crypto.Sign(sponsorPrivateKey, signBuf.Bytes())
	crcProposalWithdraw.Sign = signature

	return &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.CRCProposalWithdraw,
		Payload:    crcProposalWithdraw,
		Attributes: []*types.Attribute{},
		Inputs:     inputs,
		Outputs:    outputs,
		Programs:   []*program.Program{},
		LockTime:   0,
	}
}

func committeeKeyFrameEqual(first *CommitteeKeyFrame, second *CommitteeKeyFrame) bool {
	return keyframeEqual(first.KeyFrame, second.KeyFrame) &&
		stateKeyframeEqual(first.StateKeyFrame, second.StateKeyFrame) &&
		proposalKeyFrameEqual(first.ProposalKeyFrame, second.ProposalKeyFrame)
}

func checkResult(t *testing.T, A, B, C, D *CommitteeKeyFrame) {
	assert.Equal(t, true, committeeKeyFrameEqual(A, C))
	assert.Equal(t, false, committeeKeyFrameEqual(A, B))
	assert.Equal(t, true, committeeKeyFrameEqual(B, D))
	assert.Equal(t, false, committeeKeyFrameEqual(B, C))
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
	keyFrameA := committee.Snapshot()

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
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, common.Fixed64(0), committee.GetCandidate(*did1).votes)
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteCRTx}}, nil)
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)
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
	keyFrameA := committee.Snapshot()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 0, len(committee.GetAllMembers()))
	assert.Equal(t, 3, len(committee.GetAllCandidates()))
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)
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
	keyFrameA := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	keyFrameD := committee.Snapshot()

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
	keyFrameA2 := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 4, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameB2 := committee.Snapshot()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	keyFrameC2 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 4, len(committee.GetAllMembers()))
	assert.Equal(t, 0, len(committee.GetAllCandidates()))
	keyFrameD2 := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)
	checkResult(t, keyFrameA2, keyFrameB2, keyFrameC2, keyFrameD2)
}

func TestCommittee_RollbackChangeCommittee(t *testing.T) {
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

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2

	// avoid getting UTXOs from database
	currentHeight := cfg.CRVotingStartHeight
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
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)

	// end first voting period
	currentHeight = cfg.CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))

	// register cr again
	currentHeight = config.DefaultParams.CRCommitteeStartHeight +
		cfg.CRDutyPeriod - cfg.CRVotingPeriod
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

	// vote cr again
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}

	voteCRTx2 := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did1.Bytes(), 1},
		{did2.Bytes(), 2},
		{did3.Bytes(), 3}})
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx2,
		},
	}, nil)
	assert.Equal(t, common.Fixed64(1), committee.GetCandidate(*did1).votes)
	keyFrameA := committee.Snapshot()

	// end second voting period
	currentHeight = cfg.CRCommitteeStartHeight + cfg.CRDutyPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, common.Fixed64(1), committee.GetCandidate(*did1).votes)
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)
}

func TestCommittee_RollbackCRCProposal(t *testing.T) {
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

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2

	// avoid getting UTXOs from database
	currentHeight := cfg.CRVotingStartHeight
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
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)

	// end first voting period
	currentHeight = cfg.CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))
	keyFrameA := committee.Snapshot()

	// create CRC proposal tx
	elaAddress := "EZaqDYAPFsjynGpvHwbuiiiL4dEiHtX4gD"
	proposalTx := getCRCProposalTx(elaAddress, publicKeyStr1, privateKeyStr1,
		publicKeyStr2, privateKeyStr2)
	proposalHash := proposalTx.Payload.(*payload.CRCProposal).Hash()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposals(Registered)))
	//assert.Equal(t, 2, committee.GetProposal(proposalTx.Payload.(*payload.CRCProposal).Hash()))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 0, len(committee.GetProposals(Registered)))
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposals(Registered)))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)

	// set CR agreement count
	committee.params.CRAgreementCount = 2

	// review proposal
	proposalReviewTx1 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr1, privateKeyStr1)
	proposalReviewTx2 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr2, privateKeyStr2)
	keyFrameA2 := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalReviewTx1,
			proposalReviewTx2,
		}}, nil)
	keyFrameB2 := committee.Snapshot()
	assert.Equal(t, Registered, committee.GetProposal(proposalHash).Status)

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	keyFrameC2 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalReviewTx1,
			proposalReviewTx2,
		}}, nil)
	keyFrameD2 := committee.Snapshot()
	assert.Equal(t, Registered, committee.GetProposal(proposalHash).Status)

	checkResult(t, keyFrameA2, keyFrameB2, keyFrameC2, keyFrameD2)

	// change to CRAgreed
	keyFrameA3 := committee.Snapshot()
	currentHeight += cfg.ProposalCRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, CRAgreed, committee.GetProposal(proposalHash).Status)
	keyFrameB3 := committee.Snapshot()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, Registered, committee.GetProposal(proposalHash).Status)
	keyFrameC3 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, CRAgreed, committee.GetProposal(proposalHash).Status)
	keyFrameD3 := committee.Snapshot()

	checkResult(t, keyFrameA3, keyFrameB3, keyFrameC3, keyFrameD3)

	// change to VoterAgreed
	keyFrameA4 := committee.Snapshot()
	currentHeight += cfg.ProposalPublicVotingPeriod
	currentHeight += cfg.ProposalCRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, VoterAgreed, committee.GetProposal(proposalHash).Status)
	keyFrameB4 := committee.Snapshot()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, CRAgreed, committee.GetProposal(proposalHash).Status)
	keyFrameC4 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, VoterAgreed, committee.GetProposal(proposalHash).Status)
	keyFrameD4 := committee.Snapshot()

	checkResult(t, keyFrameA4, keyFrameB4, keyFrameC4, keyFrameD4)
}

func TestCommittee_RollbackCRCProposalTracking(t *testing.T) {
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

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2

	// avoid getting UTXOs from database
	currentHeight := cfg.CRVotingStartHeight
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
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)

	// end first voting period
	currentHeight = cfg.CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))

	// create CRC proposal tx
	elaAddress := "EZaqDYAPFsjynGpvHwbuiiiL4dEiHtX4gD"
	proposalTx := getCRCProposalTx(elaAddress, publicKeyStr1, privateKeyStr1,
		publicKeyStr2, privateKeyStr2)
	proposalHash := proposalTx.Payload.(*payload.CRCProposal).Hash()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposals(Registered)))
	//assert.Equal(t, 2, committee.GetProposal(proposalTx.Payload.(*payload.CRCProposal).Hash()))

	// set CR agreement count
	committee.params.CRAgreementCount = 2

	// review proposal
	proposalReviewTx1 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr1, privateKeyStr1)
	proposalReviewTx2 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr2, privateKeyStr2)

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalReviewTx1,
			proposalReviewTx2,
		}}, nil)
	assert.Equal(t, Registered, committee.GetProposal(proposalHash).Status)

	// change to CRAgreed
	currentHeight += cfg.ProposalCRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, CRAgreed, committee.GetProposal(proposalHash).Status)

	// change to VoterAgreed
	currentHeight += cfg.ProposalPublicVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, VoterAgreed, committee.GetProposal(proposalHash).Status)

	// set secretary-general
	publicKeyStr4 := "027209c3a6bcb95e9ef766c81136bcd6f2338eee7f9caebf694825e411320bab12"
	privateKeyStr4 := "b3b1c16abd786c4994af9ee8c79d25457f66509731f74d6a9a9673ca872fa8fa"
	committee.params.SecretaryGeneral = publicKeyStr4
	committee.getHeight = func() uint32 {
		return currentHeight
	}

	// proposal tracking of type progress
	proposalTrackingTx := getCRCProposalTrackingTx(
		payload.Progress, proposalHash, 1, publicKeyStr1, privateKeyStr1,
		"", "", privateKeyStr4)
	keyFrameA := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx,
		}}, nil)
	assert.Equal(t, 2, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 1, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx,
		}}, nil)
	assert.Equal(t, 2, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)

	// proposal tracking of type finalized
	proposalTrackingTx2 := getCRCProposalTrackingTx(
		payload.Finalized, proposalHash, 0, publicKeyStr1, privateKeyStr1,
		"", "", privateKeyStr4)
	keyFrameA2 := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx2,
		}}, nil)
	assert.Equal(t, 3, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameB2 := committee.Snapshot()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 2, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameC2 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx2,
		}}, nil)
	assert.Equal(t, 3, len(committee.GetProposal(proposalHash).WithdrawableBudgets))
	keyFrameD2 := committee.Snapshot()

	checkResult(t, keyFrameA2, keyFrameB2, keyFrameC2, keyFrameD2)
}

func TestCommittee_RollbackCRCProposalWithdraw(t *testing.T) {
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

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2

	// avoid getting UTXOs from database
	currentHeight := cfg.CRVotingStartHeight
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
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)

	// end first voting period
	currentHeight = cfg.CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))

	// create CRC proposal tx
	elaAddress := "EZaqDYAPFsjynGpvHwbuiiiL4dEiHtX4gD"
	proposalTx := getCRCProposalTx(elaAddress, publicKeyStr1, privateKeyStr1,
		publicKeyStr2, privateKeyStr2)
	proposalHash := proposalTx.Payload.(*payload.CRCProposal).Hash()
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposals(Registered)))

	// set CR agreement count
	committee.params.CRAgreementCount = 2

	// review proposal
	proposalReviewTx1 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr1, privateKeyStr1)
	proposalReviewTx2 := getCRCProposalReviewTx(proposalHash, payload.Approve,
		publicKeyStr2, privateKeyStr2)

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalReviewTx1,
			proposalReviewTx2,
		}}, nil)
	assert.Equal(t, Registered, committee.GetProposal(proposalHash).Status)

	// change to CRAgreed
	currentHeight += cfg.ProposalCRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, CRAgreed, committee.GetProposal(proposalHash).Status)

	// change to VoterAgreed
	currentHeight += cfg.ProposalPublicVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, VoterAgreed, committee.GetProposal(proposalHash).Status)

	// proposal withdraw
	withdrawTx := getCRCProposalWithdrawTx(proposalHash, publicKeyStr1,
		privateKeyStr1, 1, []*types.Input{}, []*types.Output{})
	keyFrameA := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			withdrawTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 0, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			withdrawTx,
		}}, nil)
	assert.Equal(t, 1, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)

	// set secretary-general
	publicKeyStr4 := "027209c3a6bcb95e9ef766c81136bcd6f2338eee7f9caebf694825e411320bab12"
	privateKeyStr4 := "b3b1c16abd786c4994af9ee8c79d25457f66509731f74d6a9a9673ca872fa8fa"
	committee.params.SecretaryGeneral = publicKeyStr4
	committee.getHeight = func() uint32 {
		return currentHeight
	}

	// proposal tracking of type progress
	proposalTrackingTx := getCRCProposalTrackingTx(
		payload.Progress, proposalHash, 1, publicKeyStr1, privateKeyStr1,
		"", "", privateKeyStr4)

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx,
		}}, nil)
	assert.Equal(t, 2, len(committee.GetProposal(proposalHash).WithdrawableBudgets))

	// proposal tracking of type finalized
	proposalTrackingTx2 := getCRCProposalTrackingTx(
		payload.Finalized, proposalHash, 0, publicKeyStr1, privateKeyStr1,
		"", "", privateKeyStr4)

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			proposalTrackingTx2,
		}}, nil)
	assert.Equal(t, 3, len(committee.GetProposal(proposalHash).WithdrawableBudgets))

	// proposal withdraw
	withdrawTx2 := getCRCProposalWithdrawTx(proposalHash, publicKeyStr1,
		privateKeyStr1, 1, []*types.Input{}, []*types.Output{})
	keyFrameA2 := committee.Snapshot()

	// process
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			withdrawTx2,
		}}, nil)
	assert.Equal(t, 3, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameB2 := committee.Snapshot()

	// rollback
	currentHeight--
	err = committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 1, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameC2 := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			withdrawTx2,
		}}, nil)
	assert.Equal(t, 3, len(committee.GetProposal(proposalHash).WithdrawnBudgets))
	keyFrameD2 := committee.Snapshot()

	checkResult(t, keyFrameA2, keyFrameB2, keyFrameC2, keyFrameD2)
}

func TestCommittee_RollbackTempStartVotingPeriod(t *testing.T) {
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

	// set count of CR member
	cfg := &config.DefaultParams
	cfg.CRCArbiters = cfg.CRCArbiters[0:2]
	cfg.CRMemberCount = 2
	cfg.CRAgreementCount = 2

	// avoid getting UTXOs from database
	currentHeight := cfg.CRVotingStartHeight
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
	assert.Equal(t, common.Fixed64(3), committee.GetCandidate(*did1).votes)

	// end first voting period
	currentHeight = cfg.CRCommitteeStartHeight
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, 2, len(committee.GetAllMembers()))

	currentHeight = config.DefaultParams.CRCommitteeStartHeight +
		cfg.CRDutyPeriod - cfg.CRVotingPeriod - 1
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)

	// register cr again
	currentHeight = config.DefaultParams.CRCommitteeStartHeight +
		cfg.CRDutyPeriod - cfg.CRVotingPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			registerCRTxn1,
		},
	}, nil)
	assert.Equal(t, true, committee.IsInElectionPeriod())

	// vote cr again
	for i := 0; i < 5; i++ {
		currentHeight++
		committee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: currentHeight,
			},
		}, nil)
	}
	assert.Equal(t, true, committee.IsInElectionPeriod())

	voteCRTx2 := getVoteCRTx(6, []outputpayload.CandidateVotes{
		{did1.Bytes(), 1}})
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			voteCRTx2,
		},
	}, nil)
	assert.Equal(t, common.Fixed64(1), committee.GetCandidate(*did1).votes)
	keyFrameA := committee.Snapshot()

	// end second voting period
	currentHeight = cfg.CRCommitteeStartHeight + cfg.CRDutyPeriod
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, false, committee.IsInElectionPeriod())
	assert.Equal(t, 1, len(committee.GetCandidates(Active)))
	keyFrameB := committee.Snapshot()

	// rollback
	currentHeight--
	err := committee.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, true, committee.IsInElectionPeriod())
	keyFrameC := committee.Snapshot()

	// reprocess
	currentHeight++
	committee.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	assert.Equal(t, false, committee.IsInElectionPeriod())
	assert.Equal(t, 1, len(committee.GetCandidates(Active)))
	keyFrameD := committee.Snapshot()

	checkResult(t, keyFrameA, keyFrameB, keyFrameC, keyFrameD)
}
