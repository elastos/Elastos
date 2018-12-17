package mock

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type BlockHeightMock struct {
	Producers           [][]byte
	ShouldConfirm       bool
	CurrentArbitrator   []byte
	DefaultTxVersion    byte
	DefaultBlockVersion uint32
}

func NewBlockHeightMock() *BlockHeightMock {
	const arbitratorStr = "8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62"
	arbitrator, _ := common.HexStringToBytes(arbitratorStr)

	mockObj := &BlockHeightMock{
		Producers:           make([][]byte, 0),
		ShouldConfirm:       true,
		CurrentArbitrator:   arbitrator,
		DefaultTxVersion:    1,
		DefaultBlockVersion: 1,
	}

	return mockObj
}

func (b *BlockHeightMock) GetDefaultTxVersion(blockHeight uint32) byte {
	return b.DefaultTxVersion
}

func (b *BlockHeightMock) GetDefaultBlockVersion(blockHeight uint32) uint32 {
	return b.DefaultBlockVersion
}

func (b *BlockHeightMock) CheckOutputPayload(blockHeight uint32, tx *types.Transaction, output *types.Output) error {
	return nil
}

func (b *BlockHeightMock) CheckOutputProgramHash(blockHeight uint32, tx *types.Transaction, programHash common.Uint168) error {
	return nil
}

func (b *BlockHeightMock) CheckCoinbaseMinerReward(blockHeight uint32, tx *types.Transaction, totalReward common.Fixed64) error {
	return nil
}

func (b *BlockHeightMock) CheckCoinbaseArbitratorsReward(blockHeight uint32, coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	return nil
}

func (b *BlockHeightMock) CheckVoteProducerOutputs(blockHeight uint32, tx *types.Transaction, outputs []*types.Output, references map[*types.Input]*types.Output) error {
	return nil
}

func (b *BlockHeightMock) CheckTxHasNoPrograms(blockHeight uint32, tx *types.Transaction) error {
	return nil
}

func (b *BlockHeightMock) GetProducersDesc(block *types.Block) ([][]byte, error) {
	return b.Producers, nil
}

func (b *BlockHeightMock) AddBlock(block *types.Block) (bool, bool, error) {
	return true, false, nil
}

func (b *BlockHeightMock) AddDposBlock(block *types.DposBlock) (bool, bool, error) {
	return true, false, nil
}

func (b *BlockHeightMock) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	return nil
}

func (b *BlockHeightMock) CheckConfirmedBlockOnFork(block *types.Block) error {
	return nil
}

func (b *BlockHeightMock) GetNextOnDutyArbitrator(blockHeight, dutyChangedCount, offset uint32) []byte {
	return b.CurrentArbitrator
}
