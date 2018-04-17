package transaction

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/transaction/payload"
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
