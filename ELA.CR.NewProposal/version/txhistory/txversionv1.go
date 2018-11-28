package txhistory

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/version"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type TxVersionV1 struct {
	version.TxVersionMain
}

func (v *TxVersionV1) GetVersion() byte {
	return 1
}

func (v *TxVersionV1) CheckOutputPayload(output *core.Output) error {
	return nil
}

func (v *TxVersionV1) CheckCoinbaseMinerReward(tx *core.Transaction, totalReward Fixed64) error {
	return nil
}

func (v *TxVersionV1) CheckCoinbaseArbitratorsReward(coinbase *core.Transaction, rewardInCoinbase Fixed64) error {
	return nil
}

func (v *TxVersionV1) CheckVoteProducerOutputs(outputs []*core.Output, references map[*core.Input]*core.Output) error {
	return nil
}

func (v *TxVersionV1) CheckTxHasNoProgramsAndAttributes(tx *core.Transaction) error {
	return nil
}
