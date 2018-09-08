package store

import (
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"

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

type Batch interface {
	Rollback() error
	Commit() error
}

type DataBatch interface {
	Addrs() AddrsBatch
	Txs() TxsBatch
	UTXOs() UTXOsBatch
	STXOs() STXOsBatch
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
	Get(hash *common.Uint168) (*util.Addr, error)

	// get all addresss from database
	GetAll() ([]*util.Addr, error)

	// delete a address from database
	Del(hash *common.Uint168) error
}

type AddrsBatch interface {
	Batch

	// put a address to database
	Put(hash *common.Uint168, script []byte, addrType int) error

	// delete a address from database
	Del(hash *common.Uint168) error
}

type Txs interface {
	// Put a new transaction to database
	Put(txn *util.Tx) error

	// Fetch a raw tx and it's metadata given a hash
	Get(txId *common.Uint256) (*util.Tx, error)

	// Fetch all transactions from database
	GetAll() ([]*util.Tx, error)

	// Delete a transaction from the db
	Del(txId *common.Uint256) error
}

type TxsBatch interface {
	Batch

	// Put a new transaction to database
	Put(txn *util.Tx) error

	// Delete a transaction from the db
	Del(txId *common.Uint256) error

	// Delete transactions on the given height.
	DelAll(height uint32) error
}

type UTXOs interface {
	// put a utxo to database
	Put(hash *common.Uint168, utxo *util.UTXO) error

	// get a utxo from database
	Get(op *core.OutPoint) (*util.UTXO, error)

	// get utxos of the given address hash from database
	GetAddrAll(hash *common.Uint168) ([]*util.UTXO, error)

	// Get all UTXOs in database
	GetAll() ([]*util.UTXO, error)

	// delete a utxo from database
	Del(outPoint *core.OutPoint) error
}

type UTXOsBatch interface {
	Batch

	// put a utxo to database
	Put(hash *common.Uint168, utxo *util.UTXO) error

	// delete a utxo from database
	Del(outPoint *core.OutPoint) error
}

type STXOs interface {
	// Put save a STXO into database
	Put(stxo *util.STXO) error

	// get a stxo from database
	Get(op *core.OutPoint) (*util.STXO, error)

	// get stxos of the given address hash from database
	GetAddrAll(hash *common.Uint168) ([]*util.STXO, error)

	// Get all STXOs in database
	GetAll() ([]*util.STXO, error)

	// delete a stxo from database
	Del(outPoint *core.OutPoint) error
}

type STXOsBatch interface {
	Batch

	// Put save a STXO into database
	Put(stxo *util.STXO) error

	// delete a stxo from database
	Del(outPoint *core.OutPoint) error
}
