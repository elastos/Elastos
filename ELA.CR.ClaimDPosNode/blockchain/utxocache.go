// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"container/list"
	"errors"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	maxReferenceSize = 100000
)

type UTXOCache struct {
	sync.Mutex

	db        IChainStore
	inputs    *list.List
	reference map[*types.Input]*types.Output
	txCache   map[common.Uint256]*types.Transaction
}

func (up *UTXOCache) insertReference(input *types.Input, output *types.Output) {
	if up.inputs.Len() > maxReferenceSize {
		for e := up.inputs.Front(); e != nil; e = e.Next() {
			up.inputs.Remove(e)
			delete(up.reference, e.Value.(*types.Input))
			if up.inputs.Len() <= maxReferenceSize {
				break
			}
		}
	}

	up.inputs.PushBack(input)
	up.reference[input] = output
}

func (up *UTXOCache) GetTxReference(tx *types.Transaction) (map[*types.Input]*types.Output, error) {
	up.Lock()
	defer up.Unlock()

	result := make(map[*types.Input]*types.Output)
	for _, input := range tx.Inputs {
		if output, exist := up.reference[input]; exist {
			result[input] = output
		} else {
			prevTx, err := up.getTransaction(input.Previous.TxID)
			if err != nil {
				return nil, errors.New("GetTxReference failed, " + err.Error())
			}
			if int(input.Previous.Index) >= len(prevTx.Outputs) {
				return nil, errors.New("GetTxReference failed, refIdx out of range")
			}

			result[input] = prevTx.Outputs[input.Previous.Index]
			up.insertReference(input, prevTx.Outputs[input.Previous.Index])
		}
	}

	return result, nil
}

func (up *UTXOCache) GetTransaction(txID common.Uint256) (*types.Transaction, error) {
	up.Lock()
	defer up.Unlock()

	return up.getTransaction(txID)
}

func (up *UTXOCache) getTransaction(txID common.Uint256) (*types.Transaction, error) {
	prevTx, exist := up.txCache[txID]
	if !exist {
		var err error
		prevTx, _, err = up.db.GetTransaction(txID)
		if err != nil {
			return nil, errors.New("transaction not found")
		}
		up.txCache[txID] = prevTx
	}

	return prevTx, nil
}

func (up *UTXOCache) CleanTxCache() {
	up.Lock()
	defer up.Unlock()

	up.txCache = make(map[common.Uint256]*types.Transaction)
}

func (up *UTXOCache) CleanCache() {
	up.Lock()
	defer up.Unlock()

	up.inputs.Init()
	up.reference = make(map[*types.Input]*types.Output)
	up.txCache = make(map[common.Uint256]*types.Transaction)
}

func NewUTXOCache(db IChainStore) *UTXOCache {
	return &UTXOCache{
		db:        db,
		inputs:    list.New(),
		reference: make(map[*types.Input]*types.Output),
		txCache:   make(map[common.Uint256]*types.Transaction),
	}
}
