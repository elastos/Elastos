package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type HeightVersions interface {
	GetDefaultTxVersion(blockHeight uint32) byte
	GetDefaultBlockVersion(blockHeight uint32) uint32

	AddBlock(block *types.Block) (bool, bool, error)
	AddDposBlock(block *types.DposBlock) (bool, bool, error)
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
	GetNextOnDutyArbitrator(blockHeight, dutyChangedCount, offset uint32) []byte
}
