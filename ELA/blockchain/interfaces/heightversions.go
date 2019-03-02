package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type HeightVersions interface {
	GetDefaultTxVersion(blockHeight uint32) byte
	GetDefaultBlockVersion(blockHeight uint32) uint32

	CheckOutputProgramHash(blockHeight uint32, tx *types.Transaction, programHash common.Uint168) error
	CheckCoinbaseMinerReward(blockHeight uint32, tx *types.Transaction, totalReward common.Fixed64) error
	CheckCoinbaseArbitratorsReward(blockHeight uint32, coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error

	GetNormalArbitratorsDesc(blockHeight uint32,
		arbitratorsCount uint32, arbiters []Producer) ([][]byte, error)
	GetCandidatesDesc(blockHeight uint32, startIndex uint32, producers []Producer) ([][]byte, error)
	AddBlock(block *types.Block) (bool, bool, error)
	AddDposBlock(block *types.DposBlock) (bool, bool, error)
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
	GetNextOnDutyArbitrator(blockHeight, dutyChangedCount, offset uint32) []byte
}
