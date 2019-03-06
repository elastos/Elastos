package blocks

import (
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type BlockVersion interface {
	GetVersion() uint32
	GetNormalArbitratorsDesc(arbitratorsCount uint32, arbiters []interfaces.Producer) ([][]byte, error)
	AddDposBlock(block *types.DposBlock) (bool, bool, error)
	AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error
	CheckConfirmedBlockOnFork(block *types.Block) error
	GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte
}
