package txs

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type TxVersion interface {
	GetVersion() byte
	CheckOutputPayload(txType types.TxType, output *types.Output) error
	CheckOutputProgramHash(programHash common.Uint168) error
	CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error
	CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error
	CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error
	CheckTxHasNoPrograms(tx *types.Transaction) error
}
