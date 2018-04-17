package db

import (
	. "github.com/elastos/Elastos.ELA.Utility/common"
	tx "github.com/elastos/Elastos.ELA.Utility/core/transaction"
)

type StoreTx struct {
	// Transaction ID
	TxId Uint256

	// The height at which it was mined
	Height uint32

	// Transaction
	Data tx.Transaction
}

func NewStoreTx(tx tx.Transaction, height uint32) *StoreTx {
	storeTx := new(StoreTx)
	storeTx.TxId = tx.Hash()
	storeTx.Height = height
	storeTx.Data = tx
	return storeTx
}
