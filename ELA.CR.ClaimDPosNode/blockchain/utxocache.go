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

type OutputInfo struct {
	output      *types.Output
	txtype      types.TxType
	locktime    uint32
	inputsCount int
}

type UTXOCache struct {
	sync.Mutex

	db        IChainStore
	inputs    *list.List
	reference map[*types.Input]*OutputInfo
	txCache   map[common.Uint256]*types.Transaction
}

func (up *UTXOCache) insertReference(input *types.Input, outputInfo *OutputInfo) {
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
	up.reference[input] = outputInfo
}

func (up *UTXOCache) deleteReference(input *types.Input) {
	for e := up.inputs.Front(); e != nil; e = e.Next() {
		if input == e.Value.(*types.Input) {
			up.inputs.Remove(e)
		}
	}
	delete(up.reference, input)
}

func (up *UTXOCache) GetTxReferenceInfo(tx *types.Transaction) (map[*types.Input]*OutputInfo, error) {
	up.Lock()
	defer up.Unlock()

	result := make(map[*types.Input]*OutputInfo)
	for _, input := range tx.Inputs {
		if outputInfo, exist := up.reference[input]; exist {
			result[input] = outputInfo
		} else {
			prevTx, err := up.getPrevTx(input.Previous.TxID)
			if err != nil {
				return nil, err
			}
			if int(input.Previous.Index) >= len(prevTx.Outputs) {
				return nil, errors.New("GetTxReferenceInfo failed, refIdx out of range")
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

func (up *UTXOCache) getPrevTx(txID common.Uint256) (*types.Transaction, error) {
	prevTx, exist := up.txCache[txID]
	if !exist {
		var err error
		prevTx, _, err = up.db.GetTransaction(txID)
		if err != nil {
			return nil, errors.New("GetTxReferenceInfo failed, previous transaction not found")
		}
		up.txCache[txID] = prevTx
	}

	return prevTx, nil
}

func (up *UTXOCache) CleanSpent(block *types.Block) {
	up.Lock()
	defer up.Unlock()

	for _, tx := range block.Transactions[1:] {
		for _, input := range tx.Inputs {
			delete(up.reference, input)
		}
	}
	up.txCache = make(map[common.Uint256]*types.Transaction)
}

func (up *UTXOCache) CleanCache() {
	up.Lock()
	defer up.Unlock()

	up.reference = make(map[*types.Input]*OutputInfo)
	up.txCache = make(map[common.Uint256]*types.Transaction)
}

func NewUTXOCache(db IChainStore) *UTXOCache {
	return &UTXOCache{
		db:        db,
		inputs:    list.New(),
		reference: make(map[*types.Input]*OutputInfo),
		txCache:   make(map[common.Uint256]*types.Transaction),
	}
}
