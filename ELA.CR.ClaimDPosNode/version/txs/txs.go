package txs

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type TxVersion interface {
	GetVersion() byte
	CheckOutputProgramHash(programHash common.Uint168) error
	CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error
	CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error
}
