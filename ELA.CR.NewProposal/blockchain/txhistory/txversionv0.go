package txhistory

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type TxVersionV0 struct {
	blockchain.TxVersionMain
}

func (v *TxVersionV0) GetVersion() byte {
	return 0
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
