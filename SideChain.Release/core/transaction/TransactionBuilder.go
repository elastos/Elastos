package transaction

import (
	"github.com/elastos/Elastos.ELA.SideChain/common"
	"github.com/elastos/Elastos.ELA.SideChain/core/contract/program"
	"github.com/elastos/Elastos.ELA.SideChain/core/transaction/payload"
)

func NewCoinBaseTransaction(coinBasePayload *payload.CoinBase, currentHeight uint32) (*Transaction, error) {
	return &Transaction{
		TxType:         CoinBase,
		PayloadVersion: payload.CoinBasePayloadVersion,
		Payload:        coinBasePayload,
		UTXOInputs: []*UTXOTxInput{
			{
				ReferTxID:          common.Uint256{},
				ReferTxOutputIndex: 0x0000,
				Sequence:           0x00000000,
			},
		},
		BalanceInputs: []*BalanceTxInput{},
		Attributes:    []*TxAttribute{},
		LockTime:      currentHeight,
		Programs:      []*program.Program{},
	}, nil
}
