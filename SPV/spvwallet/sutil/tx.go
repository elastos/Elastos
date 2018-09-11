package sutil

import (
	"time"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"fmt"
)

type Tx struct {
	// Transaction ID
	TxId common.Uint256

	// The height at which it was mined
	Height uint32

	// The time the transaction was first seen
	Timestamp time.Time

	// Transaction
	Data core.Transaction
}

func NewTx(tx core.Transaction, height uint32) *Tx {
	storeTx := new(Tx)
	storeTx.TxId = tx.Hash()
	storeTx.Height = height
	storeTx.Timestamp = time.Now()
	storeTx.Data = tx
	return storeTx
}

func (tx *Tx) String() string {
	return fmt.Sprintln(
		"{TxId:", tx.TxId.String(),
		", Height:", tx.Height,
		", Timestamp:", tx.Timestamp,
		"}")
}
