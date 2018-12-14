package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type HeightVersions interface {
	GetDefaultTxVersion(blockHeight uint32) byte
	GetDefaultBlockVersion(blockHeight uint32) uint32

	CheckOutputPayload(blockHeight uint32, tx *types.Transaction, output *types.Output) error
	CheckOutputProgramHash(blockHeight uint32, tx *types.Transaction, programHash common.Uint168) error
	CheckCoinbaseMinerReward(blockHeight uint32, tx *types.Transaction, totalReward common.Fixed64) error
	CheckCoinbaseArbitratorsReward(blockHeight uint32, coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error
	CheckVoteProducerOutputs(blockHeight uint32, tx *types.Transaction, outputs []*types.Output, references map[*types.Input]*types.Output) error
	CheckTxHasNoPrograms(blockHeight uint32, tx *types.Transaction) error

	GetProducersDesc(block *types.Block) ([][]byte, error)
	AddBlock(block *types.Block) error
	AddBlockConfirm(block *types.BlockConfirm) (bool, error)
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
	GetNextOnDutyArbitrator(blockHeight, dutyChangedCount, offset uint32) []byte
}
