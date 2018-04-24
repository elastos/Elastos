package db

import (
	"github.com/elastos/Elastos.ELA.SPV/db"

	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type DataStore interface {
	Info() Info
	Addrs() Addrs
	Txs() Txs
	UTXOs() UTXOs
	STXOs() STXOs

	Rollback(height uint32) error
	// Reset database, clear all data
	Reset() error

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
	Put(hash *Uint168, script []byte, addrType int) error

	// get a address from database
	Get(hash *Uint168) (*Addr, error)

	// get all addresss from database
	GetAll() ([]*Addr, error)

	// delete a address from database
	Delete(hash *Uint168) error
}

type Txs interface {
	// Put a new transaction to database
	Put(txn *db.StoreTx) error

	// Fetch a raw tx and it's metadata given a hash
	Get(txId *Uint256) (*db.StoreTx, error)

	// Fetch all transactions from database
	GetAll() ([]*db.StoreTx, error)

	// Fetch all transactions from the given height
	GetAllFrom(height uint32) ([]*db.StoreTx, error)

	// Update the height of a transaction
	UpdateHeight(txId *Uint256, height uint32) error

	// Delete a transaction from the db
	Delete(txId *Uint256) error
}

type UTXOs interface {
	// put a utxo to database
	Put(hash *Uint168, utxo *UTXO) error

	// get a utxo from database
	Get(outPoint *OutPoint) (*UTXO, error)

	// get utxos of the given address hash from database
	GetAddrAll(hash *Uint168) ([]*UTXO, error)

	// Get all UTXOs in database
	GetAll() ([]*UTXO, error)

	// delete a utxo from database
	Delete(outPoint *OutPoint) error
}

type STXOs interface {
	// Move a UTXO to STXO
	FromUTXO(outPoint *OutPoint, spendTxId *Uint256, spendHeight uint32) error

	// get a stxo from database
	Get(outPoint *OutPoint) (*STXO, error)

	// get stxos of the given address hash from database
	GetAddrAll(hash *Uint168) ([]*STXO, error)

	// Get all STXOs in database
	GetAll() ([]*STXO, error)

	// delete a stxo from database
	Delete(outPoint *OutPoint) error
}
