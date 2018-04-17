package transaction

import (
	"Elastos.ELA/common"
	"Elastos.ELA/core/contract/program"
	"Elastos.ELA/core/transaction/payload"
)

func NewCoinBaseTransaction(coinBasePayload *payload.CoinBase, currentHeight uint32) (*Transaction, error) {
	return &Transaction{
		TxType:         CoinBase,
		PayloadVersion: payload.CoinBasePayloadVersion,
		Payload:        coinBasePayload,
		Inputs: []*Input{
			{
				Previous: OutPoint{
					TxID:  common.Uint256{},
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*Attribute{},
		LockTime:   currentHeight,
		Programs:   []*program.Program{},
	}, nil
}
