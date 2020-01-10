package mempool

import (
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

const (
	txCount = 40000
)

func BenchmarkTxFeeOrderedList_AddTx(b *testing.B) {
	protoTx := types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Attributes: []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		},
	}
	txSize := protoTx.GetSize()
	orderedList := newTxFeeOrderedList(func(common.Uint256) {},
		uint64(txSize*txCount))

	for i := 0; i < txCount; i++ {
		tx := protoTx
		tx.Attributes = []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		}
		tx.Fee = common.Fixed64(rand.Int63n(1000))
		orderedList.AddTx(&tx)
	}
}

func BenchmarkTxFeeOrderedList_RemoveTx(b *testing.B) {
	protoTx := types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Attributes: []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		},
	}
	txSize := protoTx.GetSize()
	orderedList := newTxFeeOrderedList(func(common.Uint256) {},
		uint64(txSize*txCount))

	hashList := make([]common.Uint256, 0, 100)
	for i := 0; i < txCount; i++ {
		tx := protoTx
		tx.Attributes = []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		}
		tx.Fee = common.Fixed64(rand.Int63n(1000))
		orderedList.AddTx(&tx)

		hashList = append(hashList, tx.Hash())
	}

	b.ResetTimer()
	for _, v := range hashList {
		orderedList.RemoveTx(v, uint64(txSize))
	}
	b.StopTimer()
}

func BenchmarkTxFeeOrderedList_EliminateTx(b *testing.B) {
	protoTx := types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Attributes: []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		},
	}
	txSize := protoTx.GetSize()
	// size set 10000 means about 40000-30000 times eliminating action
	orderedList := newTxFeeOrderedList(func(common.Uint256) {},
		uint64(txSize*10000))

	for i := 0; i < txCount; i++ {
		tx := protoTx
		tx.Attributes = []*types.Attribute{
			{
				Usage: types.Nonce,
				Data:  randomNonceData(),
			},
		}
		tx.Fee = common.Fixed64(rand.Int63n(1000))
		orderedList.AddTx(&tx)
	}
}
