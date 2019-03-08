package heights

import (
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/blocks"
	"github.com/elastos/Elastos.ELA/version/txs"
	"github.com/stretchr/testify/suite"
)

var versionsMsg string

type heightVersionTestSuit struct {
	suite.Suite

	Version interfaces.HeightVersions
	Height1 uint32
	Height2 uint32
	Height3 uint32
}

func (s *heightVersionTestSuit) SetupTest() {
	s.Height1 = 0
	s.Height2 = 100
	s.Height3 = 200

	txV1 := &txVersionTest1{}
	txV2 := &txVersionTest2{}
	blockV1 := &txBlockTest1{}
	blockV2 := &txBlockTest2{}

	s.Version = NewHeightVersions(
		map[uint32]VersionInfo{
			s.Height1: {
				1,
				1,
				map[byte]txs.TxVersion{txV1.GetVersion(): txV1},
				map[uint32]blocks.BlockVersion{blockV1.GetVersion(): blockV1},
			},
			s.Height2: {
				2,
				2,
				map[byte]txs.TxVersion{txV1.GetVersion(): txV1, txV2.GetVersion(): txV2},
				map[uint32]blocks.BlockVersion{blockV1.GetVersion(): blockV1, blockV2.GetVersion(): blockV2},
			},
			s.Height3: {
				2,
				2,
				map[byte]txs.TxVersion{txV2.GetVersion(): txV2},
				map[uint32]blocks.BlockVersion{blockV2.GetVersion(): blockV2},
			},
		},
		s.Height3,
	)
}

func (s *heightVersionTestSuit) TestHeightVersions_GetDefaultTxVersion() {
	v := s.Version.GetDefaultTxVersion(s.Height1)
	s.Equal(byte(1), v)

	v = s.Version.GetDefaultTxVersion((s.Height1 + s.Height2) / 2)
	s.Equal(byte(1), v)

	v = s.Version.GetDefaultTxVersion(s.Height2)
	s.Equal(byte(2), v)

	v = s.Version.GetDefaultTxVersion((s.Height2 + s.Height3) / 2)
	s.Equal(byte(2), v)

	v = s.Version.GetDefaultTxVersion(s.Height3)
	s.Equal(byte(2), v)

	v = s.Version.GetDefaultTxVersion(s.Height3 + 5)
	s.Equal(byte(2), v)
}

func (s *heightVersionTestSuit) TestHeightVersions_GetDefaultBlockVersion() {
	v := s.Version.GetDefaultBlockVersion(s.Height1)
	s.Equal(uint32(1), v)

	v = s.Version.GetDefaultBlockVersion((s.Height1 + s.Height2) / 2)
	s.Equal(uint32(1), v)

	v = s.Version.GetDefaultBlockVersion(s.Height2)
	s.Equal(uint32(2), v)

	v = s.Version.GetDefaultBlockVersion((s.Height2 + s.Height3) / 2)
	s.Equal(uint32(2), v)

	v = s.Version.GetDefaultBlockVersion(s.Height3)
	s.Equal(uint32(2), v)

	v = s.Version.GetDefaultBlockVersion(s.Height3 + 5)
	s.Equal(uint32(2), v)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckConfirmedBlockOnFork() {
	var err error

	blockV1_h1 := &types.Block{Header: types.Header{Version: 1, Height: s.Height1}}
	blockV1_h2 := &types.Block{Header: types.Header{Version: 1, Height: s.Height2}}
	blockV1_h3 := &types.Block{Header: types.Header{Version: 1, Height: s.Height3}}

	err = s.Version.CheckConfirmedBlockOnFork(blockV1_h1)
	s.NoError(err)
	s.Equal("blockVersionTest1_CheckConfirmedBlockOnFork", versionsMsg)

	err = s.Version.CheckConfirmedBlockOnFork(blockV1_h2)
	s.NoError(err)
	s.Equal("blockVersionTest1_CheckConfirmedBlockOnFork", versionsMsg)

	err = s.Version.CheckConfirmedBlockOnFork(blockV1_h3)
	s.Error(err, "height 3 do not support block v1")

	blockV2_h1 := &types.Block{Header: types.Header{Version: 2, Height: s.Height1}}
	blockV2_h2 := &types.Block{Header: types.Header{Version: 2, Height: s.Height2}}
	blockV2_h3 := &types.Block{Header: types.Header{Version: 2, Height: s.Height3}}

	err = s.Version.CheckConfirmedBlockOnFork(blockV2_h1)
	s.Error(err, "height 1 do not support block v2")

	err = s.Version.CheckConfirmedBlockOnFork(blockV2_h2)
	s.NoError(err)
	s.Equal("blockVersionTest2_CheckConfirmedBlockOnFork", versionsMsg)

	err = s.Version.CheckConfirmedBlockOnFork(blockV2_h3)
	s.NoError(err)
	s.Equal("blockVersionTest2_CheckConfirmedBlockOnFork", versionsMsg)

	blockVMax_h1 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height1}}
	blockVMax_h2 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height2}}
	blockVMax_h3 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height3}}

	err = s.Version.CheckConfirmedBlockOnFork(blockVMax_h1)
	s.Error(err, "height 1 do not support block vmax")

	err = s.Version.CheckConfirmedBlockOnFork(blockVMax_h2)
	s.Error(err, "height 1 do not support block vmax")

	err = s.Version.CheckConfirmedBlockOnFork(blockVMax_h3)
	s.Error(err, "height 1 do not support block vmax")
}

func (s *heightVersionTestSuit) TestHeightVersions_AddBlockConfirmConfirm() {
	var err error

	blockV1_h1 := &types.Block{Header: types.Header{Version: 1, Height: s.Height1}}
	blockV1_h2 := &types.Block{Header: types.Header{Version: 1, Height: s.Height2}}
	blockV1_h3 := &types.Block{Header: types.Header{Version: 1, Height: s.Height3}}

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV1_h1})
	s.NoError(err)
	s.Equal("blockVersionTest1_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV1_h2})
	s.NoError(err)
	s.Equal("blockVersionTest1_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV1_h3})
	s.Error(err, "height 3 do not support block v1")

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: false, Block: blockV1_h1})
	s.Error(err, "block flag must be true")

	blockV2_h1 := &types.Block{Header: types.Header{Version: 2, Height: s.Height1}}
	blockV2_h2 := &types.Block{Header: types.Header{Version: 2, Height: s.Height2}}
	blockV2_h3 := &types.Block{Header: types.Header{Version: 2, Height: s.Height3}}

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV2_h1})
	s.Error(err, "height 1 do not support block v2")

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV2_h2})
	s.NoError(err)
	s.Equal("blockVersionTest2_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockV2_h3})
	s.NoError(err)
	s.Equal("blockVersionTest2_AddDposBlock", versionsMsg)

	blockVMax_h1 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height1}}
	blockVMax_h2 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height2}}
	blockVMax_h3 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height3}}

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockVMax_h1})
	s.Error(err, "height 1 do not support block vmax")

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockVMax_h2})
	s.Error(err, "height 1 do not support block vmax")

	_, _, err = s.Version.AddDposBlock(&types.DposBlock{BlockFlag: true, Block: blockVMax_h3})
	s.Error(err, "height 1 do not support block vmax")
}

func (s *heightVersionTestSuit) TestHeightVersions_GetNextOnDutyArbitrator() {
	s.Version.GetNextOnDutyArbitrator(s.Height1, 0, 0)
	s.Equal("blockVersionTest1_GetNextOnDutyArbitrator", versionsMsg)

	s.Version.GetNextOnDutyArbitrator(s.Height2, 0, 0)
	s.Equal("blockVersionTest2_GetNextOnDutyArbitrator", versionsMsg)

	s.Version.GetNextOnDutyArbitrator(s.Height3, 0, 0)
	s.Equal("blockVersionTest2_GetNextOnDutyArbitrator", versionsMsg)
}

func TestHeightVersionMainSuit(t *testing.T) {
	suite.Run(t, new(heightVersionTestSuit))
}

type txVersionTest1 struct {
}

func (v *txVersionTest1) GetVersion() byte {
	return 1
}

func (v *txVersionTest1) CheckOutputProgramHash(programHash common.Uint168) error {
	versionsMsg = "txVersionTest1_CheckOutputProgramHash"
	return nil
}

func (v *txVersionTest1) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error {
	versionsMsg = "txVersionTest1_CheckCoinbaseMinerReward"
	return nil
}

func (v *txVersionTest1) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	versionsMsg = "txVersionTest1_CheckCoinbaseArbitratorsReward"
	return nil
}

func (v *txVersionTest1) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error {
	versionsMsg = "txVersionTest1_CheckVoteProducerOutputs"
	return nil
}

func (v *txVersionTest1) CheckTxHasNoPrograms(tx *types.Transaction) error {
	versionsMsg = "txVersionTest1_CheckTxHasNoPrograms"
	return nil
}

type txVersionTest2 struct {
}

func (v *txVersionTest2) GetVersion() byte {
	return 2
}

func (v *txVersionTest2) CheckOutputProgramHash(programHash common.Uint168) error {
	versionsMsg = "txVersionTest2_CheckOutputProgramHash"
	return nil
}

func (v *txVersionTest2) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error {
	versionsMsg = "txVersionTest2_CheckCoinbaseMinerReward"
	return nil
}

func (v *txVersionTest2) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	versionsMsg = "txVersionTest2_CheckCoinbaseArbitratorsReward"
	return nil
}

func (v *txVersionTest2) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error {
	versionsMsg = "txVersionTest2_CheckVoteProducerOutputs"
	return nil
}

func (v *txVersionTest2) CheckTxHasNoPrograms(tx *types.Transaction) error {
	versionsMsg = "txVersionTest2_CheckTxHasNoPrograms"
	return nil
}

type txBlockTest1 struct {
}

func (v *txBlockTest1) GetVersion() uint32 {
	return 1
}

func (v *txBlockTest1) GetCandidatesDesc(startIndex uint32, producers []interfaces.Producer) ([][]byte, error) {
	versionsMsg = "blockVersionTest1_GetCandidatesDesc"
	return nil, nil
}

func (v *txBlockTest1) GetNormalArbitratorsDesc(arbitratorsCount uint32, arbiters []interfaces.Producer) (
	[][]byte, error) {
	versionsMsg = "blockVersionTest1_GetNormalArbitratorsDesc"
	return nil, nil
}

func (v *txBlockTest1) AddDposBlock(block *types.DposBlock) (bool, bool, error) {
	versionsMsg = "blockVersionTest1_AddDposBlock"
	return true, false, nil
}

func (v *txBlockTest1) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	versionsMsg = "blockVersionTest1_AssignCoinbaseTxRewards"
	return nil
}

func (v *txBlockTest1) CheckConfirmedBlockOnFork(block *types.Block) error {
	versionsMsg = "blockVersionTest1_CheckConfirmedBlockOnFork"
	return nil
}

func (v *txBlockTest1) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	versionsMsg = "blockVersionTest1_GetNextOnDutyArbitrator"
	return nil
}

type txBlockTest2 struct {
}

func (v *txBlockTest2) GetVersion() uint32 {
	return 2
}

func (v *txBlockTest2) GetCandidatesDesc(startIndex uint32, producers []interfaces.Producer) ([][]byte, error) {
	versionsMsg = "blockVersionTest2_GetCandidatesDesc"
	return nil, nil
}

func (v *txBlockTest2) GetNormalArbitratorsDesc(arbitratorsCount uint32, arbiters []interfaces.Producer) (
	[][]byte, error) {
	versionsMsg = "blockVersionTest2_GetNormalArbitratorsDesc"
	return nil, nil
}

func (v *txBlockTest2) AddDposBlock(block *types.DposBlock) (bool, bool, error) {
	versionsMsg = "blockVersionTest2_AddDposBlock"
	return true, false, nil
}

func (v *txBlockTest2) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	versionsMsg = "blockVersionTest2_AssignCoinbaseTxRewards"
	return nil
}

func (v *txBlockTest2) CheckConfirmedBlockOnFork(block *types.Block) error {
	versionsMsg = "blockVersionTest2_CheckConfirmedBlockOnFork"
	return nil
}

func (v *txBlockTest2) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	versionsMsg = "blockVersionTest2_GetNextOnDutyArbitrator"
	return nil
}
