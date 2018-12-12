package txhistory

import (
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/suite"
)

type txVersionV1TestSuite struct {
	suite.Suite

	Version version.TxVersion
}

func (s *txVersionV1TestSuite) SetupTest() {
	s.Version = &TxVersionV1{}
}
func (s *txVersionV1TestSuite) TestCheckOutputProgramHash() {
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
	s.Error(s.Version.CheckOutputProgramHash(programHash))
}

func (s *txVersionV1TestSuite) TestTxVersionMain_CheckCoinbaseMinerReward() {
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

func TestTxVersionV1Suit(t *testing.T) {
	suite.Run(t, new(txVersionV1TestSuite))
}
