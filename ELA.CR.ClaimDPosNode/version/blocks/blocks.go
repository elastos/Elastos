package blocks

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type BlockVersion interface {
	GetVersion() uint32
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
}
