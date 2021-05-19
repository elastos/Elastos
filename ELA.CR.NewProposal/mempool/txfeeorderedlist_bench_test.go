// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

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

	hashMap := make(map[common.Uint256]float64)
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

		hashMap[tx.Hash()] = float64(tx.Fee) / float64(txSize)
	}

	b.ResetTimer()
	for k, v := range hashMap {
		orderedList.RemoveTx(k, uint64(txSize), v)
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
