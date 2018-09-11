package sqlite

import (
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/sutil"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

type DataStore interface {
	State() State
	Addrs() Addrs
	Txs() Txs
	UTXOs() UTXOs
	STXOs() STXOs
	Batch() DataBatch
	Clear() error
	Close() error
}

type DataBatch interface {
	batch
	Addrs() AddrsBatch
	Txs() TxsBatch
	UTXOs() UTXOsBatch
	STXOs() STXOsBatch
	RollbackHeight(height uint32) error
}

type batch interface {
	Rollback() error
	Commit() error
}

type State interface {
	// save state height
	PutHeight(height uint32)

	// get state height
	GetHeight() uint32
}

type Addrs interface {
	// put a address to database
	Put(hash *common.Uint168, script []byte, addrType int) error

	// get a address from database
	Get(hash *common.Uint168) (*sutil.Addr, error)

	// get all addresss from database
	GetAll() ([]*sutil.Addr, error)

	// delete a address from database
	Del(hash *common.Uint168) error

	// Batch return a AddrsBatch
	Batch() AddrsBatch
}

type AddrsBatch interface {
	batch

	// put a address to database
	Put(hash *common.Uint168, script []byte, addrType int) error

	// delete a address from database
	Del(hash *common.Uint168) error
}

type Txs interface {
	// Put a new transaction to database
	Put(txn *sutil.Tx) error

	// Fetch a raw tx and it's metadata given a hash
	Get(txId *common.Uint256) (*sutil.Tx, error)

	// Fetch all transactions from database
	GetAll() ([]*sutil.Tx, error)

	// Fetch all unconfirmed transactions.
	GetAllUnconfirmed()([]*sutil.Tx, error)

	// Delete a transaction from the db
	Del(txId *common.Uint256) error

	// Batch return a TxsBatch
	Batch() TxsBatch
}

type TxsBatch interface {
	batch

	// Put a new transaction to database
	Put(txn *sutil.Tx) error

	// Delete a transaction from the db
	Del(txId *common.Uint256) error
}

type UTXOs interface {
	// put a utxo to database
	Put(utxo *sutil.UTXO) error

	// get a utxo from database
	Get(op *core.OutPoint) (*sutil.UTXO, error)

	// get utxos of the given address hash from database
	GetAddrAll(hash *common.Uint168) ([]*sutil.UTXO, error)

	// Get all UTXOs in database
	GetAll() ([]*sutil.UTXO, error)

	// delete a utxo from database
	Del(outPoint *core.OutPoint) error

	// Batch return a UTXOsBatch.
	Batch() UTXOsBatch
}

type UTXOsBatch interface {
	batch

	// put a utxo to database
	Put(utxo *sutil.UTXO) error

	// delete a utxo from database
	Del(outPoint *core.OutPoint) error
}

type STXOs interface {
	// Put save a STXO into database
	Put(stxo *sutil.STXO) error

	// get a stxo from database
	Get(op *core.OutPoint) (*sutil.STXO, error)

	// get stxos of the given address hash from database
	GetAddrAll(hash *common.Uint168) ([]*sutil.STXO, error)

	// Get all STXOs in database
	GetAll() ([]*sutil.STXO, error)

	// delete a stxo from database
	Del(outPoint *core.OutPoint) error

	// Batch return a STXOsBatch.
	Batch() STXOsBatch
}

type STXOsBatch interface {
	batch

	// Put save a STXO into database
	Put(stxo *sutil.STXO) error

	// delete a stxo from database
	Del(outPoint *core.OutPoint) error
}
