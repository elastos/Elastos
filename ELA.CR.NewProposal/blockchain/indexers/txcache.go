// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package indexers

import (
	"io"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
)

const (
	// TrimmingInterval is the interval number for each cache trimming.
	TrimmingInterval = 10000
)

type TxInfo struct {
	blockHeight uint32
	txn         *types.Transaction
}

func (t *TxInfo) Serialize(w io.Writer) (err error) {
	err = common.WriteUint32(w, t.blockHeight)
	if err != nil {
		return
	}
	return t.txn.Serialize(w)
}

func (t *TxInfo) Deserialize(r io.Reader) (err error) {
	t.blockHeight, err = common.ReadUint32(r)
	if err != nil {
		return
	}
	var txn types.Transaction
	err = txn.Deserialize(r)
	if err != nil {
		return
	}
	t.txn = &txn
	return nil
}

type TxCache struct {
	txns map[common.Uint256]*TxInfo
	sync.RWMutex

	params *config.Params
}

func (t *TxCache) Serialize(w io.Writer) (err error) {
	t.RLock()
	defer t.RUnlock()

	count := uint64(len(t.txns))
	err = common.WriteVarUint(w, count)
	if err != nil {
		return err
	}
	for _, txnInfo := range t.txns {
		err = txnInfo.Serialize(w)
		if err != nil {
			return err
		}
	}
	return nil
}

func (t *TxCache) Deserialize(r io.Reader) (err error) {
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	t.txns = make(map[common.Uint256]*TxInfo)
	for i := uint64(0); i < count; i++ {
		var txInfo TxInfo
		err = txInfo.Deserialize(r)
		if err != nil {
			return err
		}
		t.setTxn(txInfo.blockHeight, txInfo.txn)
	}

	return nil
}

func (t *TxCache) setTxn(height uint32, txn *types.Transaction) {
	if t.params.NodeProfileStrategy ==
		config.MemoryFirst.String() {
		return
	}

	t.Lock()
	defer t.Unlock()
	t.txns[txn.Hash()] = &TxInfo{
		blockHeight: height,
		txn:         txn,
	}
}

func (t *TxCache) deleteTxn(hash common.Uint256) {
	if t.params.NodeProfileStrategy ==
		config.MemoryFirst.String() {
		return
	}

	t.Lock()
	defer t.Unlock()
	delete(t.txns, hash)
}

func (t *TxCache) getTxn(hash common.Uint256) *TxInfo {
	t.RLock()
	defer t.RUnlock()

	return t.txns[hash]
}

func (t *TxCache) trim() {
	if t.params.NodeProfileStrategy ==
		config.MemoryFirst.String() {
		return
	}

	t.Lock()
	defer t.Unlock()

	trigger := t.params.TxCacheVolume + TrimmingInterval
	if len(t.txns) > int(trigger) {
		extra := len(t.txns) - int(t.params.TxCacheVolume)
		for k := range t.txns {
			delete(t.txns, k)
			extra--
			if extra < 0 {
				break
			}
		}
	}
}

func NewTxCache(params *config.Params) *TxCache {
	return &TxCache{
		txns:   make(map[common.Uint256]*TxInfo),
		params: params,
	}
}
