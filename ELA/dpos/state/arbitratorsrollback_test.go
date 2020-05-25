// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package state

import (
	"errors"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/stretchr/testify/assert"
	"testing"
)

var abt *arbitrators
var abtList [][]byte

func initArbiters() {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	abtList = make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		abtList = append(abtList, a)
	}

	activeNetParams := &config.DefaultParams
	activeNetParams.CRCArbiters = []string{
		"03e435ccd6073813917c2d841a0815d21301ec3286bc1412bb5b099178c68a10b6",
		"038a1829b4b2bee784a99bebabbfecfec53f33dadeeeff21b460f8b4fc7c2ca771",
	}
	bestHeight := uint32(0)

	abt, _ = NewArbitrators(activeNetParams,
		nil, nil)
	abt.RegisterFunction(func() uint32 { return bestHeight }, nil, nil)
	abt.State = NewState(activeNetParams, nil, nil)
}

func checkPointEqual(first, second *CheckPoint) bool {
	if !stateKeyFrameEqual(&first.StateKeyFrame, &second.StateKeyFrame) {
		return false
	}

	if first.Height != second.Height || first.DutyIndex != second.DutyIndex ||
		first.crcChangedHeight != second.crcChangedHeight ||
		first.accumulativeReward != second.accumulativeReward ||
		first.finalRoundChange != second.finalRoundChange ||
		first.clearingHeight != second.clearingHeight ||
		len(first.crcArbiters) != len(second.crcArbiters) ||
		len(first.arbitersRoundReward) != len(second.arbitersRoundReward) ||
		len(first.illegalBlocksPayloadHashes) !=
			len(second.illegalBlocksPayloadHashes) {
		return false
	}

	//	rewardEqual
	if !arrayEqual(first.CurrentArbitrators, second.CurrentArbitrators) ||
		!arrayEqual(first.NextArbitrators, second.NextArbitrators) ||
		!arrayEqual(first.CurrentCandidates, second.CurrentCandidates) ||
		!arrayEqual(first.NextCandidates, second.NextCandidates) ||
		!rewardEqual(&first.CurrentReward, &second.CurrentReward) ||
		!rewardEqual(&first.NextReward, &second.NextReward) {
		return false
	}

	for k, v := range first.crcArbiters {
		a, ok := second.crcArbiters[k]
		if !ok {
			return false
		}
		if !arbiterMemberEqual(v, a) {
			return false
		}
	}

	for k, v := range first.arbitersRoundReward {
		a, ok := second.arbitersRoundReward[k]
		if !ok {
			return false
		}
		if v != a {
			return false
		}
	}

	for k, _ := range first.illegalBlocksPayloadHashes {
		_, ok := second.illegalBlocksPayloadHashes[k]
		if !ok {
			return false
		}
	}

	return true
}

func checkResult(t *testing.T, A, B, C, D *CheckPoint) {
	assert.Equal(t, true, checkPointEqual(A, C))
	assert.Equal(t, false, checkPointEqual(A, B))
	assert.Equal(t, true, checkPointEqual(B, D))
	assert.Equal(t, false, checkPointEqual(B, C))
}

func getRegisterProducerTx(ownerPublicKey, nodePublicKey []byte,
	nickName string) *types.Transaction {
	pk, _ := crypto.DecodePoint(ownerPublicKey)
	depositCont, _ := contract.CreateDepositContractByPubKey(pk)
	return &types.Transaction{
		TxType: types.RegisterProducer,
		Payload: &payload.ProducerInfo{
			OwnerPublicKey: ownerPublicKey,
			NodePublicKey:  nodePublicKey,
			NickName: nickName,
		},
		Outputs: []*types.Output{
			{
				ProgramHash: *depositCont.ToProgramHash(),
				Value:       common.Fixed64(5000 * 1e8),
			},
		},
	}
}

func getVoteProducerTx(amount common.Fixed64,
	candidateVotes []outputpayload.CandidateVotes) *types.Transaction {
	return &types.Transaction{
		Version: 0x09,
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			&types.Output{
				AssetID:     common.Uint256{},
				Value:       amount,
				OutputLock:  0,
				ProgramHash: *randomUint168(),
				Type:        types.OTVote,
				Payload: &outputpayload.VoteOutput{
					Version: outputpayload.VoteProducerAndCRVersion,
					Contents: []outputpayload.VoteContent{
						outputpayload.VoteContent{
							VoteType:       outputpayload.Delegate,
							CandidateVotes: candidateVotes,
						},
					},
				},
			},
		},
	}
}

func getUpdateProducerTx(ownerPublicKey, nodePublicKey []byte,
	nickName string) *types.Transaction {
	return &types.Transaction{
		TxType: types.UpdateProducer,
		Payload: &payload.ProducerInfo{
			OwnerPublicKey: ownerPublicKey,
			NodePublicKey:  nodePublicKey,
			NickName:       nickName,
		},
	}
}

func getCancelProducer(publicKey []byte) *types.Transaction {
	return &types.Transaction{
		Version: 0x09,
		TxType:  types.CancelProducer,
		Payload: &payload.ProcessProducer{
			OwnerPublicKey: publicKey,
		},
	}
}

func getReturnProducerDeposit(publicKey []byte, amount common.Fixed64) *types.Transaction {
	pk, _ := crypto.DecodePoint(publicKey)
	code, _ := contract.CreateStandardRedeemScript(pk)

	return &types.Transaction{
		TxType:  types.ReturnDepositCoin,
		Payload: &payload.ReturnDepositCoin{},
		Programs: []*program.Program{
			{Code: code},
		},
		Outputs: []*types.Output{
			{Value: amount},
		},
	}
}

func TestArbitrators_RollbackRegisterProducer(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}
	arbiterStateA := abt.Snapshot()
	assert.Equal(t, 0, len(abt.PendingProducers))

	// process
	abt.ProcessBlock(block1, nil)
	arbiterStateB := abt.Snapshot()
	assert.Equal(t, 4, len(abt.PendingProducers))

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(block1, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)

	for i := uint32(0); i < 4; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.PendingProducers))
	assert.Equal(t, 0, len(abt.ActivityProducers))
	arbiterStateA2 := abt.Snapshot()

	// process
	currentHeight++
	blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
	abt.ProcessBlock(blockEx, nil)
	arbiterStateB2 := abt.Snapshot()
	assert.Equal(t, 0, len(abt.PendingProducers))
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// rollback
	currentHeight--
	err = abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC2 := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(blockEx, nil)
	arbiterStateD2 := abt.Snapshot()

	checkResult(t, arbiterStateA2, arbiterStateB2, arbiterStateC2, arbiterStateD2)
}

func TestArbitrators_RollbackVoteProducer(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
		})

	// process
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)
	arbiterStateA := abt.Snapshot()
	assert.Equal(t, common.Fixed64(5), abt.getProducer(abtList[0]).votes)

	currentHeight++
	updateProducerTx := getUpdateProducerTx(abtList[1], abtList[1], "node1")
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{updateProducerTx}}, nil)
	arbiterStateB := abt.Snapshot()

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{updateProducerTx}}, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)
}

func TestArbitrators_RollbackUpdateProducer(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
		})
	arbiterStateA := abt.Snapshot()

	// process
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)
	arbiterStateB := abt.Snapshot()
	assert.Equal(t, common.Fixed64(5), abt.getProducer(abtList[0]).votes)

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()
	assert.Equal(t, common.Fixed64(0), abt.getProducer(abtList[0]).votes)

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)
}

func TestArbitrators_RollbackCancelProducer(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
			{Candidate: abtList[1], Votes: 4},
			{Candidate: abtList[2], Votes: 3},
			{Candidate: abtList[3], Votes: 2},
		})

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)

	// cancel producer
	cancelProducerTx := getCancelProducer(abtList[0])
	arbiterStateA := abt.Snapshot()

	// process
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{cancelProducerTx}}, nil)
	arbiterStateB := abt.Snapshot()
	assert.Equal(t, 3, len(abt.GetActiveProducers()))

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()
	assert.Equal(t, 4, len(abt.GetActiveProducers()))

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{cancelProducerTx}}, nil)
	arbiterStateD := abt.Snapshot()
	assert.Equal(t, 3, len(abt.GetActiveProducers()))

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)
}

func TestArbitrators_RollbackReturnProducerDeposit(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
			{Candidate: abtList[1], Votes: 4},
			{Candidate: abtList[2], Votes: 3},
			{Candidate: abtList[3], Votes: 2},
		})

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)

	// cancel producer
	cancelProducerTx := getCancelProducer(abtList[0])

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{cancelProducerTx}}, nil)
	assert.Equal(t, 3, len(abt.GetActiveProducers()))

	// set get producer deposit amount function
	abt.getProducerDepositAmount = func(programHash common.Uint168) (
		fixed64 common.Fixed64, err error) {
		producers := abt.getAllProducers()
		for _, v := range producers {
			hash, _ := contract.PublicKeyToDepositProgramHash(
				v.info.OwnerPublicKey)
			if hash.IsEqual(programHash) {
				return v.depositAmount, nil
			}
		}

		return common.Fixed64(0), errors.New("not found producer")
	}

	// return deposit
	returnDepositTx := getReturnProducerDeposit(abtList[0], 4999*1e8)
	assert.Equal(t, common.Fixed64(5000*1e8), abt.GetProducer(abtList[0]).depositAmount)
	arbiterStateA := abt.Snapshot()

	// process
	currentHeight = abt.chainParams.CRVotingStartHeight
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{returnDepositTx}}, nil)
	assert.Equal(t, 1, len(abt.GetReturnedDepositProducers()))
	assert.Equal(t, common.Fixed64(5000*1e8), abt.GetProducer(abtList[0]).depositAmount)
	arbiterStateB := abt.Snapshot()

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 0, len(abt.GetReturnedDepositProducers()))
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{returnDepositTx}}, nil)
	assert.Equal(t, 1, len(abt.GetReturnedDepositProducers()))
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)
}

func TestArbitrators_RollbackLastBlockOfARound(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
			{Candidate: abtList[1], Votes: 4},
			{Candidate: abtList[2], Votes: 3},
			{Candidate: abtList[3], Votes: 2},
		})

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)

	// set general arbiters count
	abt.chainParams.GeneralArbiters = 2
	arbiterStateA := abt.Snapshot()

	// update next arbiters
	currentHeight = abt.chainParams.PublicDPOSHeight -
		abt.chainParams.PreConnectOffset - 1
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateB := abt.Snapshot()

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)

	// process
	arbiterStateA2 := abt.Snapshot()
	currentHeight = abt.chainParams.PublicDPOSHeight - 1
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateB2 := abt.Snapshot()

	// rollback
	currentHeight--
	err = abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC2 := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateD2 := abt.Snapshot()

	checkResult(t, arbiterStateA2, arbiterStateB2, arbiterStateC2, arbiterStateD2)

	for i := 0; i < 3; i++ {
		currentHeight++
		abt.ProcessBlock(&types.Block{
			Header: types.Header{Height: currentHeight}}, nil)
	}
	arbiterStateA3 := abt.Snapshot()

	// process
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateB3 := abt.Snapshot()

	// rollback
	currentHeight--
	err = abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC3 := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateD3 := abt.Snapshot()

	checkResult(t, arbiterStateA3, arbiterStateB3, arbiterStateC3, arbiterStateD3)
}

func TestArbitrators_RollbackRewardBlock(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
			{Candidate: abtList[1], Votes: 4},
			{Candidate: abtList[2], Votes: 3},
			{Candidate: abtList[3], Votes: 2},
		})

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)

	// set general arbiters count
	abt.chainParams.GeneralArbiters = 2

	// preConnect
	currentHeight = abt.chainParams.PublicDPOSHeight -
		abt.chainParams.PreConnectOffset - 1
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)

	currentHeight = abt.chainParams.PublicDPOSHeight - 1
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)

	for i := 0; i < 4; i++ {
		currentHeight++
		abt.ProcessBlock(&types.Block{
			Header: types.Header{Height: currentHeight}}, nil)
	}
	arbiterStateA := abt.Snapshot()

	// process reward block
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateB := abt.Snapshot()

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)

	for i := 0; i < 3; i++ {
		currentHeight++
		abt.ProcessBlock(&types.Block{
			Header: types.Header{Height: currentHeight}}, nil)
	}
	arbiterStateA2 := abt.Snapshot()

	// process reward block
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateB2 := abt.Snapshot()

	// rollback
	currentHeight--
	err = abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	arbiterStateC2 := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight}}, nil)
	arbiterStateD2 := abt.Snapshot()

	checkResult(t, arbiterStateA2, arbiterStateB2, arbiterStateC2, arbiterStateD2)
}

func TestArbitrators_RollbackMultipleTransactions(t *testing.T) {
	initArbiters()

	currentHeight := abt.chainParams.VoteStartHeight
	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			getRegisterProducerTx(abtList[0], abtList[0], "p1"),
			getRegisterProducerTx(abtList[1], abtList[1], "p2"),
			getRegisterProducerTx(abtList[2], abtList[2], "p3"),
			getRegisterProducerTx(abtList[3], abtList[3], "p4"),
		},
	}

	abt.ProcessBlock(block1, nil)

	for i := uint32(0); i < 5; i++ {
		currentHeight++
		blockEx := &types.Block{Header: types.Header{Height: currentHeight}}
		abt.ProcessBlock(blockEx, nil)
	}
	assert.Equal(t, 4, len(abt.ActivityProducers))

	// vote producer
	voteProducerTx := getVoteProducerTx(10,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[0], Votes: 5},
			{Candidate: abtList[1], Votes: 4},
			{Candidate: abtList[2], Votes: 3},
			{Candidate: abtList[3], Votes: 2},
		})

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{voteProducerTx}}, nil)

	// cancel producer
	cancelProducerTx := getCancelProducer(abtList[0])

	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header:       types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{cancelProducerTx}}, nil)
	assert.Equal(t, 3, len(abt.GetActiveProducers()))

	// set get producer deposit amount function
	abt.getProducerDepositAmount = func(programHash common.Uint168) (
		fixed64 common.Fixed64, err error) {
		producers := abt.getAllProducers()
		for _, v := range producers {
			hash, _ := contract.PublicKeyToDepositProgramHash(
				v.info.OwnerPublicKey)
			if hash.IsEqual(programHash) {
				return v.depositAmount, nil
			}
		}

		return common.Fixed64(0), errors.New("not found producer")
	}

	registerProducerTx2 := getRegisterProducerTx(abtList[4], abtList[4], "p5")
	voteProducerTx2 := getVoteProducerTx(2,
		[]outputpayload.CandidateVotes{
			{Candidate: abtList[1], Votes: 1},
		})
	updateProducerTx2 := getUpdateProducerTx(abtList[1], abtList[1], "node1")
	cancelProducerTx2 := getCancelProducer(abtList[2])
	returnDepositTx2 := getReturnProducerDeposit(abtList[0], 4999*1e8)
	assert.Equal(t, common.Fixed64(5000*1e8), abt.GetProducer(abtList[0]).depositAmount)

	arbiterStateA := abt.Snapshot()

	// process
	currentHeight = abt.chainParams.CRVotingStartHeight
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			registerProducerTx2,
			voteProducerTx2,
			updateProducerTx2,
			cancelProducerTx2,
			returnDepositTx2,
		}}, nil)
	assert.Equal(t, 2, len(abt.GetActiveProducers()))
	assert.Equal(t, 1, len(abt.GetReturnedDepositProducers()))
	assert.Equal(t, common.Fixed64(5000*1e8), abt.GetProducer(abtList[0]).depositAmount)
	arbiterStateB := abt.Snapshot()

	// rollback
	currentHeight--
	err := abt.RollbackTo(currentHeight)
	assert.NoError(t, err)
	assert.Equal(t, 3, len(abt.GetActiveProducers()))
	assert.Equal(t, 0, len(abt.GetReturnedDepositProducers()))
	arbiterStateC := abt.Snapshot()

	// reprocess
	currentHeight++
	abt.ProcessBlock(&types.Block{
		Header: types.Header{Height: currentHeight},
		Transactions: []*types.Transaction{
			registerProducerTx2,
			voteProducerTx2,
			updateProducerTx2,
			cancelProducerTx2,
			returnDepositTx2,
		}}, nil)
	assert.Equal(t, 1, len(abt.GetReturnedDepositProducers()))
	arbiterStateD := abt.Snapshot()

	checkResult(t, arbiterStateA, arbiterStateB, arbiterStateC, arbiterStateD)
}
