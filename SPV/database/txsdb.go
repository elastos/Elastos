package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

// TxsDB stores all transactions in main chain and fork chains.
type TxsDB interface {
	// Extend from DB interface
	DB

	// PutTxs persists the main chain transactions into database and can be
	// queried by GetTxs(height).  Returns the false positive transaction count
	// and error.
	PutTxs(txs []util.Transaction, height uint32) (uint32, error)

	// PutForkTxs persists the fork chain transactions into database with the
	// fork block hash and can be queried by GetForkTxs(hash).
	PutForkTxs(txs []util.Transaction, hash *common.Uint256) error

	// HaveTx returns if the transaction already saved in database
	// by it's id.
	HaveTx(txId *common.Uint256) (bool, error)

	// GetTxs returns all transactions in main chain within the given height.
	GetTxs(height uint32) ([]util.Transaction, error)

	// GetForkTxs returns all transactions within the fork block hash.
	GetForkTxs(hash *common.Uint256) ([]util.Transaction, error)

	// DelTxs remove all transactions in main chain within the given height.
	DelTxs(height uint32) error
}
