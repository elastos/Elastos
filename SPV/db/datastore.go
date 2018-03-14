package db

import (
	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/bloom"
)

type DataStore interface {
	Scripts() Scripts
	UTXOs() UTXOs
	STXOs() STXOs
	TXNs() TXNs

	GetFilter() *bloom.Filter

	Close()
}

type Scripts interface {
	// put a script to database
	Put(hash *Uint168, script []byte) error

	// get a script from database
	Get(hash *Uint168) ([]byte, error)

	// get all scripts from database
	GetAll() ([][]byte, error)

	// delete a script from database
	Delete(hash *Uint168) error

	// get scripts filter
	GetFilter() *ScriptFilter
}

type UTXOs interface {
	// put a utxo to database
	Put(hash *Uint168, utxo *UTXO) error

	// get a utxo from database
	Get(outPoint *tx.OutPoint) (*UTXO, error)

	// get utxos of the given script hash from database
	GetAddrAll(hash *Uint168) ([]*UTXO, error)

	// Get all UTXOs in database
	GetAll() ([]*UTXO, error)

	// delete a utxo from database
	Delete(outPoint *tx.OutPoint) error
}

type STXOs interface {
	// Move a UTXO to STXO
	FromUTXO(outPoint *tx.OutPoint, spendHeight uint32, spendTxId *Uint256) error

	// get a stxo from database
	Get(outPoint *tx.OutPoint) (*STXO, error)

	// get stxos of the given script hash from database
	GetAddrAll(hash *Uint168) ([]*STXO, error)

	// Get all STXOs in database
	GetAll() ([]*STXO, error)

	// delete a stxo from database
	Delete(outPoint *tx.OutPoint) error
}

type TXNs interface {
	// Put a new transaction to database
	Put(txn *Txn) error

	// Fetch a raw tx and it's metadata given a hash
	Get(txId *Uint256) (*Txn, error)

	// Fetch all transactions from database
	GetAll() ([]*Txn, error)

	// Fetch all transactions from the given height
	GetAllFrom(height uint32) ([]*Txn, error)

	// Update the height of a transaction
	UpdateHeight(txId *Uint256, height uint32) error

	// Delete a transaction from the db
	Delete(txId *Uint256) error
}
