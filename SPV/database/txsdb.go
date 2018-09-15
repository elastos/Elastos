package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

type TxsDB interface {
	// Extend from DB interface
	DB

	// Batch returns a TxBatch instance for transactions batch
	// commit, this can get better performance when commit a bunch
	// of transactions within a block.
	Batch() TxBatch

	// HaveTx returns if the transaction already saved in database
	// by it's id.
	HaveTx(txId *common.Uint256) (bool, error)

	// GetTxs returns all transactions within the given height.
	GetTxs(height uint32) ([]*util.Tx, error)

	// RemoveTxs delete all transactions on the given height.  Return
	// how many transactions are deleted from database.
	RemoveTxs(height uint32) (int, error)
}

type TxBatch interface {
	// PutTx add a store transaction operation into batch, and return
	// if it is a false positive and error.
	PutTx(tx *util.Tx) (bool, error)

	// DelTx add a delete transaction operation into batch.
	DelTx(txId *common.Uint256) error

	// DelTxs add a delete transactions on given height operation.
	DelTxs(height uint32) error

	// Rollback cancel all operations in current batch.
	Rollback() error

	// Commit the added transactions into database.
	Commit() error
}
