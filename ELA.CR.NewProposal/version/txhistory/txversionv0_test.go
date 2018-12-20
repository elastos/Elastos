package txhistory

import (
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/suite"
)

type txVersionV0TestSuite struct {
	suite.Suite

	Version version.TxVersion
}

func (s *txVersionV0TestSuite) SetupTest() {
	s.Version = &TxVersionV0{}
}

func (s *txVersionV0TestSuite) TestCheckOutputProgramHash() {
	programHash := common.Uint168{}

	// empty program hash should pass
	s.NoError(s.Version.CheckOutputProgramHash(programHash))

	// prefix standard program hash should pass
	programHash[0] = uint8(contract.PrefixStandard)
	s.NoError(s.Version.CheckOutputProgramHash(programHash))

	// prefix multisig program hash should pass
	programHash[0] = uint8(contract.PrefixMultiSig)
	s.NoError(s.Version.CheckOutputProgramHash(programHash))

	// prefix crosschain program hash should pass
	programHash[0] = uint8(contract.PrefixCrossChain)
	s.NoError(s.Version.CheckOutputProgramHash(programHash))

	// other prefix program hash should not pass
	programHash[0] = 0x34
	s.NoError(s.Version.CheckOutputProgramHash(programHash))
}

func (s *txVersionV0TestSuite) TestCheckCoinbaseMinerReward() {
	totalReward := blockchain.RewardAmountPerBlock
	tx := &types.Transaction{
		Version: types.TransactionVersion(s.Version.GetVersion()),
		TxType:  types.CoinBase,
	}
	// reward to foundation in coinbase = 30%, reward to miner in coinbase >= 35%
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	minerReward := common.Fixed64(float64(totalReward) * 0.35)
	dposReward := totalReward - foundationReward - minerReward
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: foundationReward},
		{ProgramHash: common.Uint168{}, Value: minerReward},
		{ProgramHash: common.Uint168{}, Value: dposReward},
	}
	err := s.Version.CheckCoinbaseMinerReward(tx, totalReward)
	s.NoError(err)

	// reward to foundation in coinbase = 30%, reward to miner in coinbase < 35%
	foundationReward = common.Fixed64(float64(totalReward) * 0.3)
	minerReward = common.Fixed64(float64(totalReward) * 0.3499999)
	dposReward = totalReward - foundationReward - minerReward
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: foundationReward},
		{ProgramHash: common.Uint168{}, Value: minerReward},
		{ProgramHash: common.Uint168{}, Value: dposReward},
	}
	err = s.Version.CheckCoinbaseMinerReward(tx, totalReward)
	s.NoError(err, "Reward to miner in coinbase < 35%")
}

func (s *txVersionV0TestSuite) TestCheckCoinbaseArbitratorsReward() {
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

	for _, v := range arbitratorsStr {
		a, _ := common.HexStringToBytes(v)
		arbitrators = append(arbitrators, a)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		arbitratorHashes = append(arbitratorHashes, hash)
	}
	fmt.Println()
	for _, v := range candidatesStr {
		a, _ := common.HexStringToBytes(v)
		candidates = append(candidates, a)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		candidateHashes = append(candidateHashes, hash)
	}

	originLedger := blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Arbitrators: &mock.ArbitratorsMock{
			CurrentArbitrators:         arbitrators,
			CurrentCandidates:          candidates,
			CurrentArbitratorsPrograms: arbitratorHashes,
			CurrentCandidatesPrograms:  candidateHashes,
		},
	}

	rewardInCoinbase := common.Fixed64(1000)
	dposTotalReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(5+5)))
	tx := &types.Transaction{
		Version: types.TransactionVersion(s.Version.GetVersion()),
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: blockchain.FoundationAddress, Value: common.Fixed64(float64(rewardInCoinbase) * 0.30)},
		{ProgramHash: common.Uint168{}, Value: common.Fixed64(float64(rewardInCoinbase) * 0.35)},
	}

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(tx, rewardInCoinbase))

	for _, v := range arbitratorHashes {
		tx.Outputs = append(tx.Outputs, &types.Output{ProgramHash: *v, Value: individualBlockConfirmReward + individualProducerReward})
	}
	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(tx, rewardInCoinbase))

	for _, v := range candidateHashes {
		tx.Outputs = append(tx.Outputs, &types.Output{ProgramHash: *v, Value: individualProducerReward})
	}
	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(tx, rewardInCoinbase))

	blockchain.DefaultLedger = originLedger
}

func (s *txVersionV0TestSuite) TestCheckVoteProducerOutputs() {
	outputs := []*types.Output{
		{
			OutputType: types.DefaultOutput,
		},
	}
	references := make(map[*types.Input]*types.Output)

	s.NoError(s.Version.CheckVoteProducerOutputs(outputs, references, nil))

	hashStr := "21c5656c65028fe21f2222e8f0cd46a1ec734cbdb6"
	hashByte, _ := common.HexStringToBytes(hashStr)
	hash, _ := common.Uint168FromBytes(hashByte)
	outputs = append(outputs, &types.Output{
		OutputType:  types.VoteOutput,
		ProgramHash: *hash,
	})
	s.NoError(s.Version.CheckVoteProducerOutputs(outputs, references, nil))

	references[&types.Input{}] = &types.Output{
		ProgramHash: *hash,
	}
	s.NoError(s.Version.CheckVoteProducerOutputs(outputs, references, nil))
}

func (s *txVersionV0TestSuite) TestCheckTxHasNoPrograms() {
	tx := &types.Transaction{
		Version:  types.TransactionVersion(s.Version.GetVersion()),
		TxType:   types.CoinBase,
		Programs: make([]*program.Program, 0),
	}

	s.NoError(s.Version.CheckTxHasNoPrograms(tx))

	tx.Programs = append(tx.Programs, &program.Program{})
	s.NoError(s.Version.CheckTxHasNoPrograms(tx))
}

func TestTxVersionV0Suit(t *testing.T) {
	suite.Run(t, new(txVersionV0TestSuite))
}
