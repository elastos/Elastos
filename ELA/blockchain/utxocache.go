// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

type OutputInfo struct {
	output      *types.Output
	txtype      types.TxType
	locktime    uint32
	inputsCount int
}

type UTXOCache struct {
	sync.Mutex

	db        IChainStore
	reference map[*types.Input]*OutputInfo
}

func (up *UTXOCache) GetTxReferenceInfo(tx *types.Transaction) (map[*types.Input]*OutputInfo, error) {
	up.Lock()
	defer up.Unlock()

	prevTxCache := make(map[common.Uint256]*types.Transaction)
	result := make(map[*types.Input]*OutputInfo)
	for _, input := range tx.Inputs {
		if outputInfo, exist := up.reference[input]; exist {
			result[input] = outputInfo
		} else {
			prevTx, exist := prevTxCache[input.Previous.TxID]
			if !exist {
				var err error
				prevTx, _, err = up.db.GetTransaction(input.Previous.TxID)
				if err != nil {
					return nil, errors.New("GetTxReferenceInfo failed, " +
						"previous transaction not found")
				}
			}
			if int(input.Previous.Index) >= len(prevTx.Outputs) {
				return nil, errors.New("GetTxReferenceInfo failed, " +
					"refIdx out of range")
			}
			refer := &OutputInfo{
				output:      prevTx.Outputs[input.Previous.Index],
				txtype:      prevTx.TxType,
				locktime:    prevTx.LockTime,
				inputsCount: len(prevTx.Inputs),
			}

			result[input] = refer
			up.reference[input] = refer
		}
	}

	return result, nil
}

func (up *UTXOCache) CleanSpentUTXOs(block *types.Block) {
	up.Lock()
	defer up.Unlock()

	for _, tx := range block.Transactions[1:] {
		for _, input := range tx.Inputs {
			delete(up.reference, input)
		}
	}
}

func (up *UTXOCache) CleanCache() {
	up.Lock()
	defer up.Unlock()

	up.reference = make(map[*types.Input]*OutputInfo)
}

func NewUTXOCache(db IChainStore) *UTXOCache {
	return &UTXOCache{
		db:        db,
		reference: make(map[*types.Input]*OutputInfo),
	}
}
