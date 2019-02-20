package blocks

import (
	"bytes"
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/version/verconf"
	"github.com/stretchr/testify/assert"
)

var version BlockVersion
var cfg *verconf.Config
var arbitratorsMock *mock.ArbitratorsMock
var arbitrators [][]byte
var originLedger *blockchain.Ledger

func TestBlockVersionInit(t *testing.T) {
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	chainStore, err := blockchain.NewChainStore("blockVersionTestSuite",
		config.MainNetParams.GenesisBlock)
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
	config.Parameters.ArbiterConfiguration.CRCArbiters = []config.CRCArbitratorConfigItem{
		{PublicKey: "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a"},
		{PublicKey: "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"},
		{PublicKey: "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"},
		{PublicKey: "03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd"},
		{PublicKey: "0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0"},
	}

	arbitrators = make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
	}
	arbitratorsMock = &mock.ArbitratorsMock{
		CurrentArbitrators: arbitrators,
	}

	chain, err := blockchain.New(chainStore, &config.MainNetParams,
		arbitratorsMock, nil)
	if err != nil {
		t.Error(err)
	}

	cfg = &verconf.Config{
		Chain:       chain,
		Arbitrators: arbitratorsMock,
	}
	version = NewBlockV2(cfg)

	originLedger = blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: arbitratorsMock,
		Blockchain:  &blockchain.BlockChain{},
	}
}

func TestBlockVersionMain_GetNormalArbitratorsDesc(t *testing.T) {

	currentHeight := uint32(1)

	block1 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitrators[0],
					NodePublicKey:  arbitrators[0],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitrators[1],
					NodePublicKey:  arbitrators[1],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitrators[2],
					NodePublicKey:  arbitrators[2],
				},
			},
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitrators[3],
					NodePublicKey:  arbitrators[3],
				},
			},
		},
	}
	cfg.Chain.GetState().ProcessBlock(block1, nil)

	for i := uint32(0); i < 6; i++ {
		currentHeight++
		blockEx := &types.Block{
			Header:       types.Header{Height: currentHeight},
			Transactions: []*types.Transaction{},
		}
		cfg.Chain.GetState().ProcessBlock(blockEx, nil)
	}

	producers, err := version.GetNormalArbitratorsDesc(5)
	assert.Error(t, err, "arbitrators count does not match config value")

	currentHeight += 1
	block2 := &types.Block{
		Header: types.Header{
			Height: currentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: arbitrators[4],
					NodePublicKey:  arbitrators[4],
				},
			},
		},
	}
	cfg.Chain.GetState().ProcessBlock(block2, nil)

	for i := uint32(0); i < 6; i++ {
		currentHeight++
		blockEx := &types.Block{
			Header:       types.Header{Height: currentHeight},
			Transactions: []*types.Transaction{},
		}
		cfg.Chain.GetState().ProcessBlock(blockEx, nil)
	}

	producers, err = version.GetNormalArbitratorsDesc(5)
	assert.NoError(t, err)
	for i := range producers {
		found := false
		for _, ar := range arbitrators {
			if bytes.Equal(ar, producers[i]) {
				found = true
				break
			}
		}

		assert.True(t, found)
	}
}

func TestBlockVersionMain_AssignCoinbaseTxRewards(t *testing.T) {
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
	candidates := make([][]byte, 0)
	arbitratorHashes := make([]*common.Uint168, 0)
	candidateHashes := make([]*common.Uint168, 0)
	arbitratorHashMap := make(map[common.Uint168]interface{})
	candidateHashMap := make(map[common.Uint168]interface{})

	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		arbitratorHashes = append(arbitratorHashes, hash)
		arbitratorHashMap[*hash] = nil
	}
	fmt.Println()
	for _, v := range candidatesStr {
		a, _ := common.HexStringToBytes(v)
		candidates = append(candidates, a)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		candidateHashes = append(candidateHashes, hash)
		candidateHashMap[*hash] = nil
	}

	arbitratorsMock.CurrentCandidates = candidates
	arbitratorsMock.CurrentArbitratorsPrograms = arbitratorHashes
	arbitratorsMock.CurrentCandidatesPrograms = candidateHashes

	//reward can be exactly division

	rewardInCoinbase := common.Fixed64(1000)
	foundationReward := common.Fixed64(float64(rewardInCoinbase) * 0.3)
	minerReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	dposTotalReward := rewardInCoinbase - foundationReward - minerReward
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(5+5)))
	arbitratorsChange := dposTotalReward - (individualProducerReward+individualBlockConfirmReward)*5 - individualProducerReward*5

	tx := &types.Transaction{
		Version: types.TransactionVersion(version.GetVersion()),
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: 0},
		{ProgramHash: common.Uint168{}, Value: 0},
	}
	block := &types.Block{
		Transactions: []*types.Transaction{
			tx,
		},
	}

	assert.NoError(t, version.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	assert.Equal(t, foundationReward, tx.Outputs[0].Value)
	assert.NotEqual(t, minerReward, tx.Outputs[1].Value)
	assert.Equal(t, minerReward+arbitratorsChange, tx.Outputs[1].Value,
		"should add change of arbitrators' reward")
	assert.Equal(t, 2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			assert.Equal(t, individualBlockConfirmReward+individualProducerReward,
				tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			assert.Equal(t, individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		assert.Equal(t, true, found)
	}

	//reward can not be exactly division

	rewardInCoinbase = common.Fixed64(999)
	foundationReward = common.Fixed64(math.Ceil(float64(rewardInCoinbase) * 0.3))
	foundationRewardNormal := common.Fixed64(float64(rewardInCoinbase) * 0.3)
	minerReward = common.Fixed64(float64(rewardInCoinbase) * 0.35)
	dposTotalReward = rewardInCoinbase - foundationReward - minerReward
	totalBlockConfirmReward = float64(dposTotalReward) * 0.25
	totalTopProducersReward = float64(dposTotalReward) * 0.75
	individualBlockConfirmReward = common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	individualProducerReward = common.Fixed64(math.Floor(totalTopProducersReward / float64(5+5)))
	arbitratorsChange = dposTotalReward - (individualProducerReward+individualBlockConfirmReward)*5 - individualProducerReward*5

	tx = &types.Transaction{
		Version: types.TransactionVersion(version.GetVersion()),
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

	assert.NoError(t, version.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	assert.NotEqual(t, foundationRewardNormal, tx.Outputs[0].Value)
	assert.Equal(t, foundationReward, tx.Outputs[0].Value)
	assert.Equal(t, minerReward+arbitratorsChange, tx.Outputs[1].Value)
	assert.Equal(t, 2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			assert.Equal(t,
				individualBlockConfirmReward+individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			assert.Equal(t, individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		assert.Equal(t, true, found)
	}
}

func TestBlockVersionMain_GetNextOnDutyArbitrator(t *testing.T) {
	var currentArbitrator []byte

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 0)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(1, 0)
	assert.Equal(t, arbitrators[1], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(2, 0)
	assert.Equal(t, arbitrators[2], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(3, 0)
	assert.Equal(t, arbitrators[3], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(4, 0)
	assert.Equal(t, arbitrators[4], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(5, 0)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 1)
	assert.Equal(t, arbitrators[1], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 2)
	assert.Equal(t, arbitrators[2], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 3)
	assert.Equal(t, arbitrators[3], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 4)
	assert.Equal(t, arbitrators[4], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 5)
	assert.Equal(t, arbitrators[0], currentArbitrator)

	currentArbitrator = version.GetNextOnDutyArbitrator(0, 6)
	assert.Equal(t, arbitrators[1], currentArbitrator)
}

func TestBlockVersionDone(t *testing.T) {
	blockchain.DefaultLedger = originLedger
}
