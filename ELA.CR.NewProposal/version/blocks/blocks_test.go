package blocks

import (
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/suite"
)

type blockVersionTestSuite struct {
	suite.Suite

	Version BlockVersion
}

func (s *blockVersionTestSuite) SetupTest() {
	s.Version = &blockV2{}
}

func (s *blockVersionTestSuite) TestGetNormalArbitratorsDesc(
	arbitratorsCount uint32) {
	originLedger := blockchain.DefaultLedger

	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	arbitrators := make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
	}

	chainStore := &blockchain.ChainStoreMock{
		RegisterProducers: []*payload.ProducerInfo{
			{
				OwnerPublicKey: arbitrators[0],
			},
			{
				OwnerPublicKey: arbitrators[1],
			},
			{
				OwnerPublicKey: arbitrators[2],
			},
			{
				OwnerPublicKey: arbitrators[3],
			},
		},
	}
	s.NotEmpty(chainStore)
	blockchain.DefaultLedger = &blockchain.Ledger{
		Store: chainStore,
	}

	producers, err := s.Version.GetNormalArbitratorsDesc()
	s.Error(err, "arbitrators count does not match config value")

	chainStore.RegisterProducers = append(chainStore.RegisterProducers,
		&payload.ProducerInfo{OwnerPublicKey: arbitrators[4]},
	)
	producers, err = s.Version.GetNormalArbitratorsDesc()
	s.NoError(err)
	for i := range producers {
		s.Equal(arbitrators[i], producers[i])
	}

	blockchain.DefaultLedger = originLedger
}

func (s *blockVersionTestSuite) TestAssignCoinbaseTxRewards() {
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

	originLedger := blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: &mock.ArbitratorsMock{
			CurrentArbitrators:         arbitrators,
			CurrentCandidates:          candidates,
			CurrentArbitratorsPrograms: arbitratorHashes,
			CurrentCandidatesPrograms:  candidateHashes,
		},
		Blockchain: &blockchain.BlockChain{},
	}

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
		Version: types.TransactionVersion(s.Version.GetVersion()),
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

	s.NoError(s.Version.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	s.Equal(foundationReward, tx.Outputs[0].Value)
	s.NotEqual(minerReward, tx.Outputs[1].Value)
	s.Equal(minerReward+arbitratorsChange, tx.Outputs[1].Value, "should add change of arbitrators' reward")
	s.Equal(2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			s.Equal(individualBlockConfirmReward+individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			s.Equal(individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		s.Equal(true, found)
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
		Version: types.TransactionVersion(s.Version.GetVersion()),
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

	s.NoError(s.Version.AssignCoinbaseTxRewards(block, rewardInCoinbase))

	s.NotEqual(foundationRewardNormal, tx.Outputs[0].Value)
	s.Equal(foundationReward, tx.Outputs[0].Value)
	s.Equal(minerReward+arbitratorsChange, tx.Outputs[1].Value)
	s.Equal(2+5+5, len(tx.Outputs))
	for i := 2; i < 12; i++ {
		found := false
		if _, ok := arbitratorHashMap[tx.Outputs[i].ProgramHash]; ok {
			s.Equal(individualBlockConfirmReward+individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		if _, ok := candidateHashMap[tx.Outputs[i].ProgramHash]; ok {
			s.Equal(individualProducerReward, tx.Outputs[i].Value)
			found = true
		}

		s.Equal(true, found)
	}

	blockchain.DefaultLedger = originLedger
}

func (s *blockVersionTestSuite) TestBlockVersionMain_GetNextOnDutyArbitrator() {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}

	arbitrators := make([][]byte, 0)
	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
	}

	originLedger := blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: &mock.ArbitratorsMock{
			CurrentArbitrators: arbitrators,
		},
	}

	var currentArbitrator []byte

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 0)
	s.Equal(arbitrators[0], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(1, 0)
	s.Equal(arbitrators[1], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(2, 0)
	s.Equal(arbitrators[2], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(3, 0)
	s.Equal(arbitrators[3], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(4, 0)
	s.Equal(arbitrators[4], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(5, 0)
	s.Equal(arbitrators[0], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 1)
	s.Equal(arbitrators[1], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 2)
	s.Equal(arbitrators[2], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 3)
	s.Equal(arbitrators[3], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 4)
	s.Equal(arbitrators[4], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 5)
	s.Equal(arbitrators[0], currentArbitrator)

	currentArbitrator = s.Version.GetNextOnDutyArbitrator(0, 6)
	s.Equal(arbitrators[1], currentArbitrator)

	blockchain.DefaultLedger = originLedger
}

func TestBlockVersionSuit(t *testing.T) {
	suite.Run(t, new(blockVersionTestSuite))
}
