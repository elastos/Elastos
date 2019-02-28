package txs

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV1 implement the TxVersion interface.
var _ TxVersion = (*txV0)(nil)

type txV0 struct {
	cfg *verconf.Config
}

func (v *txV0) GetVersion() byte {
	return 0
}

func (v *txV0) CheckOutputPayload(txType types.TxType, output *types.Output) error {
	return nil
}

func (v *txV0) CheckOutputProgramHash(programHash common.Uint168) error {
	return nil
}

func (v *txV0) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error {
	return nil
}

func (v *txV0) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	return nil
}

func (v *txV0) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error {
	return nil
}

func (v *txV0) CheckTxHasNoPrograms(tx *types.Transaction) error {
	return nil
}

func NewTxV0(cfg *verconf.Config) *txV0 {
	return &txV0{cfg: cfg}
}
