package txhistory

import (
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA/version"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type TxVersionV0 struct {
	version.TxVersionMain
}

func (v *TxVersionV0) GetVersion() byte {
	return 0
}

func (v *TxVersionV0) CheckOutputPayload(output *core.Output) error {
	return nil
}

func (v *TxVersionV0) CheckOutputProgramHash(programHash Uint168) error {
	return nil
}

func (v *TxVersionV0) CheckCoinbaseMinerReward(tx *core.Transaction, totalReward Fixed64) error {
	return nil
}

func (v *TxVersionV0) CheckCoinbaseArbitratorsReward(coinbase *core.Transaction, rewardInCoinbase Fixed64) error {
	return nil
}

func (v *TxVersionV0) CheckVoteProducerOutputs(outputs []*core.Output, references map[*core.Input]*core.Output) error {
	return nil
}

func (v *TxVersionV0) CheckTxHasNoProgramsAndAttributes(tx *core.Transaction) error {
	return nil
}
