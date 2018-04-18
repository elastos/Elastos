package db

import (
	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type StoreTx struct {
	// Transaction ID
	TxId Uint256

	// The height at which it was mined
	Height uint32

	// Transaction
	Data Transaction
}

func NewStoreTx(tx Transaction, height uint32) *StoreTx {
	storeTx := new(StoreTx)
	storeTx.TxId = tx.Hash()
	storeTx.Height = height
	storeTx.Data = tx
	return storeTx
}
