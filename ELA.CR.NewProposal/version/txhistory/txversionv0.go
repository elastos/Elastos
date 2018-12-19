package txhistory

import (
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/version"

	. "github.com/elastos/Elastos.ELA/common"
)

type TxVersionV0 struct {
	version.TxVersionMain
}

func (v *TxVersionV0) GetVersion() byte {
	return 0
}

func (v *TxVersionV0) CheckOutputPayload(txType types.TransactionType, output *types.Output) error {
	return nil
}

func (v *TxVersionV0) CheckOutputProgramHash(programHash Uint168) error {
	return nil
}

func (v *TxVersionV0) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward Fixed64) error {
	return nil
}

func (v *TxVersionV0) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase Fixed64) error {
	return nil
}

func (v *TxVersionV0) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output) error {
	return nil
}

func (v *TxVersionV0) CheckTxHasNoPrograms(tx *types.Transaction) error {
	return nil
}
