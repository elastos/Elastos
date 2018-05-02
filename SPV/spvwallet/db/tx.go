package db

import (
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type Tx struct {
	// Transaction ID
	TxId Uint256

	// The height at which it was mined
	Height uint32

	// Transaction
	Data Transaction
}

func NewTx(tx Transaction, height uint32) *Tx {
	storeTx := new(Tx)
	storeTx.TxId = tx.Hash()
	storeTx.Height = height
	storeTx.Data = tx
	return storeTx
}
