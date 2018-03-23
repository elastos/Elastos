package db

import (
	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/bloom"
)

type DataStore interface {
	Info() Info
	Addrs() Addrs
	UTXOs() UTXOs
	STXOs() STXOs
	Proofs() Proofs
	TXNs() TXNs

	Rollback(height uint32) error
	GetFilter() *bloom.Filter

	Close()
}

type Info interface {
	// get chain height
	ChainHeight() uint32

	// save chain height
	SaveChainHeight(height uint32)

	// put key and value into db
	Put(key string, data []byte) error

	// get value by key
	Get(key string) ([]byte, error)

	// delete value by key
	Delete(key string) error
}

type Addrs interface {
	// put a address to database
	Put(hash *Uint168, script []byte) error

	// get a address from database
	Get(hash *Uint168) (*Addr, error)

	// get all addresss from database
	GetAll() ([]*Addr, error)

	// delete a address from database
	Delete(hash *Uint168) error

	// get addresss filter
	GetFilter() *AddrFilter
}

type UTXOs interface {
	// put a utxo to database
	Put(hash *Uint168, utxo *UTXO) error

	// get a utxo from database
	Get(outPoint *tx.OutPoint) (*UTXO, error)

	// get utxos of the given address hash from database
	GetAddrAll(hash *Uint168) ([]*UTXO, error)

	// Get all UTXOs in database
	GetAll() ([]*UTXO, error)

	// delete a utxo from database
	Delete(outPoint *tx.OutPoint) error
}

type STXOs interface {
	// Move a UTXO to STXO
	FromUTXO(outPoint *tx.OutPoint, spendTxId *Uint256, spendHeight uint32) error

	// get a stxo from database
	Get(outPoint *tx.OutPoint) (*STXO, error)

	// get stxos of the given address hash from database
	GetAddrAll(hash *Uint168) ([]*STXO, error)

	// Get all STXOs in database
	GetAll() ([]*STXO, error)

	// delete a stxo from database
	Delete(outPoint *tx.OutPoint) error
}

type Proofs interface {
	// Put a merkle proof of the block
	Put(proof *Proof) error

	// Get a merkle proof of a block
	Get(blockHash *Uint256) (*Proof, error)

	// Get all merkle proofs in database
	GetAll() ([]*Proof, error)

	// Delete a merkle proof of a block
	Delete(blockHash *Uint256) error
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
