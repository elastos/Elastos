// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package pow

import (
	"fmt"
	"math"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var pow *Service
var arbitratorsMock *state.ArbitratorsMock
var arbitrators []state.ArbiterMember
var originLedger *blockchain.Ledger

func TestService_Init(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	params := &config.DefaultParams
	chainStore, err := blockchain.NewChainStore(filepath.Join(test.DataPath, "service"), params)
	if err != nil {
		t.Error(err)
	}

	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	params.CRCArbiters = []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	arbitrators = make([]state.ArbiterMember, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		arbitrators = append(arbitrators, ar)
	}
	arbitratorsMock = &state.ArbitratorsMock{
		CurrentArbitrators: arbitrators,
	}

	chain, err := blockchain.New(chainStore, params, state.NewState(params,
		nil, nil), nil)
	if err != nil {
		t.Error(err)
	}

	originLedger = blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: arbitratorsMock,
		Blockchain:  &blockchain.BlockChain{},
	}

	pow = NewService(&Config{
		Chain:       chain,
		Arbitrators: arbitratorsMock,
		ChainParams: params,
	})
}

func TestService_AssignCoinbaseTxRewards(t *testing.T) {
	// main version >= H2
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	candidatesStr := []string{
		"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
		"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
		"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
		"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
		"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
	}

	arbitrators := make([][]byte, 0)
	candidates := make([]state.ArbiterMember, 0)
	arbitratorHashes := make([]*common.Uint168, 0)
	candidateHashes := make([]*common.Uint168, 0)
	arbitratorHashMap := make(map[common.Uint168]interface{})
	candidateHashMap := make(map[common.Uint168]interface{})
	ownerVotes := make(map[common.Uint168]common.Fixed64)
	totalVotesInRound := 0

	for i, v := range arbitratorsStr {
		vote := i + 10
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		arbitratorHashes = append(arbitratorHashes, hash)
		arbitratorHashMap[*hash] = nil
		ownerVotes[*hash] = common.Fixed64(vote)
		totalVotesInRound += vote
	}
	fmt.Println()
	for i, v := range candidatesStr {
		vote := i + 1
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		candidates = append(candidates, ar)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		candidateHashes = append(candidateHashes, hash)
		candidateHashMap[*hash] = nil
		ownerVotes[*hash] = common.Fixed64(vote)
		totalVotesInRound += vote
	}

	arbitratorsMock.CurrentCandidates = candidates
	arbitratorsMock.CurrentOwnerProgramHashes = arbitratorHashes
	arbitratorsMock.CandidateOwnerProgramHashes = candidateHashes
	arbitratorsMock.OwnerVotesInRound = ownerVotes
	arbitratorsMock.TotalVotesInRound = common.Fixed64(totalVotesInRound)

	//reward can be exactly division
	rewardInCoinbase := common.Fixed64(1000)
	foundationReward := common.Fixed64(float64(rewardInCoinbase) * 0.3)
	dposTotalReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	minerReward := rewardInCoinbase - foundationReward - dposTotalReward
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)
	realReward := common.Fixed64(0)
	arbitratorsMock.ArbitersRoundReward = map[common.Uint168]common.Fixed64{}
	for hash, vote := range ownerVotes {
		individualProducerReward := common.Fixed64(rewardPerVote * float64(vote))
		if _, ok := arbitratorHashMap[hash]; ok {
			realReward = realReward + individualProducerReward + individualBlockConfirmReward
			arbitratorsMock.ArbitersRoundReward[hash] = individualProducerReward + individualBlockConfirmReward
		}

		if _, ok := candidateHashMap[hash]; ok {
			realReward = realReward + individualProducerReward
			arbitratorsMock.ArbitersRoundReward[hash] = individualProducerReward
		}
	}
	arbitratorsChange := dposTotalReward - realReward
	arbitratorsMock.FinalRoundChange = arbitratorsChange

	tx := &types.Transaction{
		Version: types.TxVersion09,
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: 0},
		{ProgramHash: common.Uint168{}, Value: 0},
	}
	block := &types.Block{
		Header: types.Header{
			Height: config.DefaultParams.PublicDPOSHeight,
		},
		Transactions: []*types.Transaction{
			tx,
		},
	}

	assert.NoError(t, pow.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	assert.Equal(t, foundationReward, tx.Outputs[0].Value)
	assert.NotEqual(t, minerReward, tx.Outputs[1].Value)
	assert.Equal(t, minerReward+arbitratorsChange, tx.Outputs[1].Value,
		"should add change of arbitrators' reward")
	assert.Equal(t, 2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			vote := ownerVotes[tx.Outputs[i].ProgramHash]
			individualProducerReward := common.Fixed64(float64(vote) * rewardPerVote)
			assert.Equal(t, individualBlockConfirmReward+individualProducerReward,
				tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			vote := ownerVotes[tx.Outputs[i].ProgramHash]
			individualProducerReward := common.Fixed64(float64(vote) * rewardPerVote)
			assert.Equal(t, individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		assert.Equal(t, true, found)
	}

	//reward can not be exactly division

	rewardInCoinbase = common.Fixed64(999)
	foundationReward = common.Fixed64(math.Ceil(float64(rewardInCoinbase) * 0.3))
	foundationRewardNormal := common.Fixed64(float64(rewardInCoinbase) * 0.3)
	dposTotalReward = common.Fixed64(math.Ceil(float64(rewardInCoinbase) * 0.35))
	minerReward = rewardInCoinbase - foundationReward - dposTotalReward
	totalBlockConfirmReward = float64(dposTotalReward) * 0.25
	totalTopProducersReward = float64(dposTotalReward) - totalBlockConfirmReward
	individualBlockConfirmReward = common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	rewardPerVote = totalTopProducersReward / float64(totalVotesInRound)
	realReward = common.Fixed64(0)
	arbitratorsMock.ArbitersRoundReward = map[common.Uint168]common.Fixed64{}
	for hash, vote := range ownerVotes {
		individualProducerReward := common.Fixed64(rewardPerVote * float64(vote))
		if _, ok := arbitratorHashMap[hash]; ok {
			realReward = realReward + individualProducerReward + individualBlockConfirmReward
			arbitratorsMock.ArbitersRoundReward[hash] = individualProducerReward + individualBlockConfirmReward
		}

		if _, ok := candidateHashMap[hash]; ok {
			realReward = realReward + individualProducerReward
			arbitratorsMock.ArbitersRoundReward[hash] = individualProducerReward
		}
	}
	arbitratorsChange = dposTotalReward - realReward
	arbitratorsMock.FinalRoundChange = arbitratorsChange

	tx = &types.Transaction{
		Version: types.TxVersion09,
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: 0},
		{ProgramHash: common.Uint168{}, Value: 0},
	}
	block = &types.Block{
		Header: types.Header{
			Height: config.DefaultParams.PublicDPOSHeight,
		},
		Transactions: []*types.Transaction{
			tx,
		},
	}

	assert.NoError(t, pow.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	assert.NotEqual(t, foundationRewardNormal, tx.Outputs[0].Value)
	assert.Equal(t, foundationReward, tx.Outputs[0].Value)
	assert.Equal(t, minerReward+arbitratorsChange, tx.Outputs[1].Value)
	assert.Equal(t, 2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			vote := ownerVotes[tx.Outputs[i].ProgramHash]
			individualProducerReward := common.Fixed64(float64(vote) * rewardPerVote)
			assert.Equal(t,
				individualBlockConfirmReward+individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			vote := ownerVotes[tx.Outputs[i].ProgramHash]
			individualProducerReward := common.Fixed64(float64(vote) * rewardPerVote)
			assert.Equal(t, individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		assert.Equal(t, true, found)
	}

	// version [0,H2)
	originLedger := blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Blockchain: &blockchain.BlockChain{},
	}
	//reward can be exactly division
	rewardInCoinbase = common.Fixed64(1000)
	foundationReward = common.Fixed64(float64(rewardInCoinbase) * 0.3)
	minerReward = common.Fixed64(float64(rewardInCoinbase) * 0.35)
	dposTotalReward = rewardInCoinbase - foundationReward - minerReward

	tx = &types.Transaction{
		Version: 0,
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: 0},
		{ProgramHash: common.Uint168{}, Value: 0},
	}
	block = &types.Block{
		Transactions: []*types.Transaction{
			tx,
		},
	}

	assert.NoError(t, pow.AssignCoinbaseTxRewards(block, rewardInCoinbase))
	assert.Equal(t, foundationReward, tx.Outputs[0].Value)
	assert.Equal(t, minerReward, tx.Outputs[1].Value)
	assert.Equal(t, dposTotalReward, tx.Outputs[1].Value)

	//reward can not be exactly division
	rewardInCoinbase = common.Fixed64(999)
	foundationReward = common.Fixed64(math.Ceil(float64(rewardInCoinbase) * 0.3))
	foundationRewardNormal = common.Fixed64(float64(rewardInCoinbase) * 0.3)
	minerReward = common.Fixed64(float64(rewardInCoinbase) * 0.35)
	dposTotalReward = rewardInCoinbase - foundationRewardNormal - minerReward

	tx = &types.Transaction{
		Version: 0,
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: 0},
		{ProgramHash: common.Uint168{}, Value: 0},
	}
	block = &types.Block{
		Transactions: []*types.Transaction{
			tx,
		},
	}

	assert.NoError(t, pow.AssignCoinbaseTxRewards(block, rewardInCoinbase))
	assert.Equal(t, foundationRewardNormal, tx.Outputs[0].Value)
	assert.NotEqual(t, foundationReward, tx.Outputs[0].Value)
	assert.Equal(t, minerReward, tx.Outputs[1].Value)
	assert.Equal(t, dposTotalReward, tx.Outputs[2].Value)

	blockchain.DefaultLedger = originLedger
}
