package version

import (
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/heights"
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
	s.Height1 = heights.GenesisHeightVersion
	s.Height2 = heights.HeightVersion1
	s.Height3 = heights.HeightVersion2

	txV1 := &txVersionTest1{}
	txV2 := &txVersionTest2{}
	blockV1 := &txBlockTest1{}
	blockV2 := &txBlockTest2{}

	s.Version = NewHeightVersions(
		map[uint32]VersionInfo{
			s.Height1: {
				1,
				1,
				map[byte]TxVersion{txV1.GetVersion(): txV1},
				map[uint32]BlockVersion{blockV1.GetVersion(): blockV1},
			},
			s.Height2: {
				2,
				2,
				map[byte]TxVersion{txV1.GetVersion(): txV1, txV2.GetVersion(): txV2},
				map[uint32]BlockVersion{blockV1.GetVersion(): blockV1, blockV2.GetVersion(): blockV2},
			},
			s.Height3: {
				2,
				2,
				map[byte]TxVersion{txV2.GetVersion(): txV2},
				map[uint32]BlockVersion{blockV2.GetVersion(): blockV2},
			},
		},
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

func (s *heightVersionTestSuit) TestHeightVersions_CheckOutputPayload() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckOutputPayload(s.Height1, txV1, nil))
	s.Equal("txVersionTest1_CheckOutputPayload", versionsMsg)

	s.NoError(s.Version.CheckOutputPayload(s.Height1, txV2, nil))
	s.Equal("txVersionTest1_CheckOutputPayload", versionsMsg)

	s.NoError(s.Version.CheckOutputPayload(s.Height1, txVMax, nil))
	s.Equal("txVersionTest1_CheckOutputPayload", versionsMsg)

	s.Error(s.Version.CheckOutputPayload(s.Height1, nil, nil), "do not support nil tx")

	s.NoError(s.Version.CheckOutputPayload((s.Height1+s.Height2)/2, txV1, nil))
	s.Equal("txVersionTest1_CheckOutputPayload", versionsMsg)

	s.NoError(s.Version.CheckOutputPayload(s.Height2, txV1, nil))
	s.Equal("txVersionTest2_CheckOutputPayload", versionsMsg)

	s.NoError(s.Version.CheckOutputPayload((s.Height2+s.Height3)/2, txV1, nil))
	s.Equal("txVersionTest2_CheckOutputPayload", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckOutputPayload(s.Height3, txV1, nil), "do not support v1")
	s.Error(s.Version.CheckOutputPayload(s.Height3, txVMax, nil), "do not support vMax")
	s.Error(s.Version.CheckOutputPayload(s.Height3, nil, nil), "do not support nil tx")

	s.NoError(s.Version.CheckOutputPayload(s.Height3, txV2, nil))
	s.Equal("txVersionTest2_CheckOutputPayload", versionsMsg)

	s.NoError(s.Version.CheckOutputPayload(s.Height3+5, txV2, nil))
	s.Equal("txVersionTest2_CheckOutputPayload", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckOutputProgramHash() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckOutputProgramHash(s.Height1, txV1, common.Uint168{}))
	s.Equal("txVersionTest1_CheckOutputProgramHash", versionsMsg)

	s.NoError(s.Version.CheckOutputProgramHash(s.Height1, txV2, common.Uint168{}))
	s.Equal("txVersionTest1_CheckOutputProgramHash", versionsMsg)

	s.NoError(s.Version.CheckOutputProgramHash(s.Height1, txVMax, common.Uint168{}))
	s.Equal("txVersionTest1_CheckOutputProgramHash", versionsMsg)

	s.Error(s.Version.CheckOutputProgramHash(s.Height1, nil, common.Uint168{}), "do not support nil tx")

	s.NoError(s.Version.CheckOutputProgramHash((s.Height1+s.Height2)/2, txV1, common.Uint168{}))
	s.Equal("txVersionTest1_CheckOutputProgramHash", versionsMsg)

	s.NoError(s.Version.CheckOutputProgramHash(s.Height2, txV1, common.Uint168{}))
	s.Equal("txVersionTest2_CheckOutputProgramHash", versionsMsg)

	s.NoError(s.Version.CheckOutputProgramHash((s.Height2+s.Height3)/2, txV1, common.Uint168{}))
	s.Equal("txVersionTest2_CheckOutputProgramHash", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckOutputProgramHash(s.Height3, txV1, common.Uint168{}), "do not support v1")
	s.Error(s.Version.CheckOutputProgramHash(s.Height3, txVMax, common.Uint168{}), "do not support vMax")
	s.Error(s.Version.CheckOutputProgramHash(s.Height3, nil, common.Uint168{}), "do not support nil tx")

	s.NoError(s.Version.CheckOutputProgramHash(s.Height3, txV2, common.Uint168{}))
	s.Equal("txVersionTest2_CheckOutputProgramHash", versionsMsg)

	s.NoError(s.Version.CheckOutputProgramHash(s.Height3+5, txV2, common.Uint168{}))
	s.Equal("txVersionTest2_CheckOutputProgramHash", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckCoinbaseMinerReward() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height1, txV1, 0))
	s.Equal("txVersionTest1_CheckCoinbaseMinerReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height1, txV2, 0))
	s.Equal("txVersionTest1_CheckCoinbaseMinerReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height1, txVMax, 0))
	s.Equal("txVersionTest1_CheckCoinbaseMinerReward", versionsMsg)

	s.Error(s.Version.CheckCoinbaseMinerReward(s.Height1, nil, 0), "do not support nil tx")

	s.NoError(s.Version.CheckCoinbaseMinerReward((s.Height1+s.Height2)/2, txV1, 0))
	s.Equal("txVersionTest1_CheckCoinbaseMinerReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height2, txV1, 0))
	s.Equal("txVersionTest2_CheckCoinbaseMinerReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseMinerReward((s.Height2+s.Height3)/2, txV1, 0))
	s.Equal("txVersionTest2_CheckCoinbaseMinerReward", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckCoinbaseMinerReward(s.Height3, txV1, 0), "do not support v1")
	s.Error(s.Version.CheckCoinbaseMinerReward(s.Height3, txVMax, 0), "do not support vMax")
	s.Error(s.Version.CheckCoinbaseMinerReward(s.Height3, nil, 0), "do not support nil tx")

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height3, txV2, 0))
	s.Equal("txVersionTest2_CheckCoinbaseMinerReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseMinerReward(s.Height3+5, txV2, 0))
	s.Equal("txVersionTest2_CheckCoinbaseMinerReward", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckCoinbaseArbitratorsReward() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height1, txV1, 0))
	s.Equal("txVersionTest1_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height1, txV2, 0))
	s.Equal("txVersionTest1_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height1, txVMax, 0))
	s.Equal("txVersionTest1_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.Error(s.Version.CheckCoinbaseArbitratorsReward(s.Height1, nil, 0), "do not support nil tx")

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward((s.Height1+s.Height2)/2, txV1, 0))
	s.Equal("txVersionTest1_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height2, txV1, 0))
	s.Equal("txVersionTest2_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward((s.Height2+s.Height3)/2, txV1, 0))
	s.Equal("txVersionTest2_CheckCoinbaseArbitratorsReward", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckCoinbaseArbitratorsReward(s.Height3, txV1, 0), "do not support v1")
	s.Error(s.Version.CheckCoinbaseArbitratorsReward(s.Height3, txVMax, 0), "do not support vMax")
	s.Error(s.Version.CheckCoinbaseArbitratorsReward(s.Height3, nil, 0), "do not support nil tx")

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height3, txV2, 0))
	s.Equal("txVersionTest2_CheckCoinbaseArbitratorsReward", versionsMsg)

	s.NoError(s.Version.CheckCoinbaseArbitratorsReward(s.Height3+5, txV2, 0))
	s.Equal("txVersionTest2_CheckCoinbaseArbitratorsReward", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckVoteProducerOutputs() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height1, txV1, nil, nil))
	s.Equal("txVersionTest1_CheckVoteProducerOutputs", versionsMsg)

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height1, txV2, nil, nil))
	s.Equal("txVersionTest1_CheckVoteProducerOutputs", versionsMsg)

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height1, txVMax, nil, nil))
	s.Equal("txVersionTest1_CheckVoteProducerOutputs", versionsMsg)

	s.Error(s.Version.CheckVoteProducerOutputs(s.Height1, nil, nil, nil), "do not support nil tx")

	s.NoError(s.Version.CheckVoteProducerOutputs((s.Height1+s.Height2)/2, txV1, nil, nil))
	s.Equal("txVersionTest1_CheckVoteProducerOutputs", versionsMsg)

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height2, txV1, nil, nil))
	s.Equal("txVersionTest2_CheckVoteProducerOutputs", versionsMsg)

	s.NoError(s.Version.CheckVoteProducerOutputs((s.Height2+s.Height3)/2, txV1, nil, nil))
	s.Equal("txVersionTest2_CheckVoteProducerOutputs", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckVoteProducerOutputs(s.Height3, txV1, nil, nil), "do not support v1")
	s.Error(s.Version.CheckVoteProducerOutputs(s.Height3, txVMax, nil, nil), "do not support vMax")
	s.Error(s.Version.CheckVoteProducerOutputs(s.Height3, nil, nil, nil), "do not support nil tx")

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height3, txV2, nil, nil))
	s.Equal("txVersionTest2_CheckVoteProducerOutputs", versionsMsg)

	s.NoError(s.Version.CheckVoteProducerOutputs(s.Height3+5, txV2, nil, nil))
	s.Equal("txVersionTest2_CheckVoteProducerOutputs", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_CheckTxHasNoPrograms() {
	txV1 := &types.Transaction{Version: 1}
	txV2 := &types.Transaction{Version: 2}
	txVMax := &types.Transaction{Version: 255}

	//note less or equal than heights.HeightVersion2(s.Height2) find version only by DefaultTxVersion

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height1, txV1))
	s.Equal("txVersionTest1_CheckTxHasNoPrograms", versionsMsg)

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height1, txV2))
	s.Equal("txVersionTest1_CheckTxHasNoPrograms", versionsMsg)

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height1, txVMax))
	s.Equal("txVersionTest1_CheckTxHasNoPrograms", versionsMsg)

	s.Error(s.Version.CheckTxHasNoPrograms(s.Height1, nil), "do not support nil tx")

	s.NoError(s.Version.CheckTxHasNoPrograms((s.Height1+s.Height2)/2, txV1))
	s.Equal("txVersionTest1_CheckTxHasNoPrograms", versionsMsg)

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height2, txV1))
	s.Equal("txVersionTest2_CheckTxHasNoPrograms", versionsMsg)

	s.NoError(s.Version.CheckTxHasNoPrograms((s.Height2+s.Height3)/2, txV1))
	s.Equal("txVersionTest2_CheckTxHasNoPrograms", versionsMsg)

	//greater than heights.HeightVersion2(s.Height2) find version by Transaction.Version

	s.Error(s.Version.CheckTxHasNoPrograms(s.Height3, txV1), "do not support v1")
	s.Error(s.Version.CheckTxHasNoPrograms(s.Height3, txVMax), "do not support vMax")
	s.Error(s.Version.CheckTxHasNoPrograms(s.Height3, nil), "do not support nil tx")

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height3, txV2))
	s.Equal("txVersionTest2_CheckTxHasNoPrograms", versionsMsg)

	s.NoError(s.Version.CheckTxHasNoPrograms(s.Height3+5, txV2))
	s.Equal("txVersionTest2_CheckTxHasNoPrograms", versionsMsg)
}

func (s *heightVersionTestSuit) TestHeightVersions_GetProducersDesc() {
	var err error

	blockV1_h1 := &types.Block{Header: types.Header{Version: 1, Height: s.Height1}}
	blockV1_h2 := &types.Block{Header: types.Header{Version: 1, Height: s.Height2}}
	blockV1_h3 := &types.Block{Header: types.Header{Version: 1, Height: s.Height3}}

	_, err = s.Version.GetProducersDesc(blockV1_h1)
	s.NoError(err)
	s.Equal("blockVersionTest1_GetProducersDesc", versionsMsg)

	_, err = s.Version.GetProducersDesc(blockV1_h2)
	s.NoError(err)
	s.Equal("blockVersionTest1_GetProducersDesc", versionsMsg)

	_, err = s.Version.GetProducersDesc(blockV1_h3)
	s.Error(err, "height 3 do not support block v1")

	blockV2_h1 := &types.Block{Header: types.Header{Version: 2, Height: s.Height1}}
	blockV2_h2 := &types.Block{Header: types.Header{Version: 2, Height: s.Height2}}
	blockV2_h3 := &types.Block{Header: types.Header{Version: 2, Height: s.Height3}}

	_, err = s.Version.GetProducersDesc(blockV2_h1)
	s.Error(err, "height 1 do not support block v2")

	_, err = s.Version.GetProducersDesc(blockV2_h2)
	s.NoError(err)
	s.Equal("blockVersionTest2_GetProducersDesc", versionsMsg)

	_, err = s.Version.GetProducersDesc(blockV2_h3)
	s.NoError(err)
	s.Equal("blockVersionTest2_GetProducersDesc", versionsMsg)

	blockVMax_h1 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height1}}
	blockVMax_h2 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height2}}
	blockVMax_h3 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height3}}

	_, err = s.Version.GetProducersDesc(blockVMax_h1)
	s.Error(err, "height 1 do not support block vmax")

	_, err = s.Version.GetProducersDesc(blockVMax_h2)
	s.Error(err, "height 1 do not support block vmax")

	_, err = s.Version.GetProducersDesc(blockVMax_h3)
	s.Error(err, "height 1 do not support block vmax")
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

func (s *heightVersionTestSuit) TestHeightVersions_AddBlock() {
	var err error

	blockV1_h1 := &types.Block{Header: types.Header{Version: 1, Height: s.Height1}}
	blockV1_h2 := &types.Block{Header: types.Header{Version: 1, Height: s.Height2}}
	blockV1_h3 := &types.Block{Header: types.Header{Version: 1, Height: s.Height3}}

	_, _, err = s.Version.AddBlock(blockV1_h1)
	s.NoError(err)
	s.Equal("blockVersionTest1_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddBlock(blockV1_h2)
	s.NoError(err)
	s.Equal("blockVersionTest1_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddBlock(blockV1_h3)
	s.Error(err, "height 3 do not support block v1")

	blockV2_h1 := &types.Block{Header: types.Header{Version: 2, Height: s.Height1}}
	blockV2_h2 := &types.Block{Header: types.Header{Version: 2, Height: s.Height2}}
	blockV2_h3 := &types.Block{Header: types.Header{Version: 2, Height: s.Height3}}

	_, _, err = s.Version.AddBlock(blockV2_h1)
	s.Error(err, "height 1 do not support block v2")

	_, _, err = s.Version.AddBlock(blockV2_h2)
	s.NoError(err)
	s.Equal("blockVersionTest2_AddDposBlock", versionsMsg)

	_, _, err = s.Version.AddBlock(blockV2_h3)
	s.NoError(err)
	s.Equal("blockVersionTest2_AddDposBlock", versionsMsg)

	blockVMax_h1 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height1}}
	blockVMax_h2 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height2}}
	blockVMax_h3 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height3}}

	_, _, err = s.Version.AddBlock(blockVMax_h1)
	s.Error(err, "height 1 do not support block vmax")

	_, _, err = s.Version.AddBlock(blockVMax_h2)
	s.Error(err, "height 1 do not support block vmax")

	_, _, err = s.Version.AddBlock(blockVMax_h3)
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

func (s *heightVersionTestSuit) TestHeightVersions_AssignCoinbaseTxRewards() {
	var err error

	blockV1_h1 := &types.Block{Header: types.Header{Version: 1, Height: s.Height1}}
	blockV1_h2 := &types.Block{Header: types.Header{Version: 1, Height: s.Height2}}
	blockV1_h3 := &types.Block{Header: types.Header{Version: 1, Height: s.Height3}}

	err = s.Version.AssignCoinbaseTxRewards(blockV1_h1, 0)
	s.NoError(err)
	s.Equal("blockVersionTest1_AssignCoinbaseTxRewards", versionsMsg)

	err = s.Version.AssignCoinbaseTxRewards(blockV1_h2, 0)
	s.NoError(err)
	s.Equal("blockVersionTest1_AssignCoinbaseTxRewards", versionsMsg)

	err = s.Version.AssignCoinbaseTxRewards(blockV1_h3, 0)
	s.Error(err, "height 3 do not support block v1")

	blockV2_h1 := &types.Block{Header: types.Header{Version: 2, Height: s.Height1}}
	blockV2_h2 := &types.Block{Header: types.Header{Version: 2, Height: s.Height2}}
	blockV2_h3 := &types.Block{Header: types.Header{Version: 2, Height: s.Height3}}

	err = s.Version.AssignCoinbaseTxRewards(blockV2_h1, 0)
	s.Error(err, "height 1 do not support block v2")

	err = s.Version.AssignCoinbaseTxRewards(blockV2_h2, 0)
	s.NoError(err)
	s.Equal("blockVersionTest2_AssignCoinbaseTxRewards", versionsMsg)

	err = s.Version.AssignCoinbaseTxRewards(blockV2_h3, 0)
	s.NoError(err)
	s.Equal("blockVersionTest2_AssignCoinbaseTxRewards", versionsMsg)

	blockVMax_h1 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height1}}
	blockVMax_h2 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height2}}
	blockVMax_h3 := &types.Block{Header: types.Header{Version: math.MaxUint32, Height: s.Height3}}

	err = s.Version.AssignCoinbaseTxRewards(blockVMax_h1, 0)
	s.Error(err, "height 1 do not support block vmax")

	err = s.Version.AssignCoinbaseTxRewards(blockVMax_h2, 0)
	s.Error(err, "height 1 do not support block vmax")

	err = s.Version.AssignCoinbaseTxRewards(blockVMax_h3, 0)
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

func (v *txVersionTest1) CheckOutputPayload(output *types.Output) error {
	versionsMsg = "txVersionTest1_CheckOutputPayload"
	return nil
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

func (v *txVersionTest1) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output) error {
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

func (v *txVersionTest2) CheckOutputPayload(output *types.Output) error {
	versionsMsg = "txVersionTest2_CheckOutputPayload"
	return nil
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

func (v *txVersionTest2) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output) error {
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

func (v *txBlockTest1) GetProducersDesc() ([][]byte, error) {
	versionsMsg = "blockVersionTest1_GetProducersDesc"
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

func (v *txBlockTest2) GetProducersDesc() ([][]byte, error) {
	versionsMsg = "blockVersionTest2_GetProducersDesc"
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
