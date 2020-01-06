// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package indexers

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
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
}

func (t *TxCache) Serialize(w io.Writer) (err error) {
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
	t.txns[txn.Hash()] = &TxInfo{
		blockHeight: height,
		txn:         txn,
	}
}

func (t *TxCache) deleteTxn(hash common.Uint256) {
	delete(t.txns, hash)
}

func (t *TxCache) getTxn(hash common.Uint256) *TxInfo {
	return t.txns[hash]
}

func NewTxCache() *TxCache {
	return &TxCache{
		txns: make(map[common.Uint256]*TxInfo),
	}
}
