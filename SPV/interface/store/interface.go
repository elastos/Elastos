package store

import (
	"time"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

type HeaderStore interface {
	database.Headers
	GetByHeight(height uint32) (header *util.Header, err error)
}

type DataStore interface {
	database.DB
	Addrs() Addrs
	Txs() Txs
	Ops() Ops
	Que() Que
	Batch() DataBatch
}

type DataBatch interface {
	batch
	Txs() TxsBatch
	Ops() OpsBatch
	Que() QueBatch
	// Delete all transactions, ops, queued items on
	// the given height.
	DelAll(height uint32) error
}

type batch interface {
	Rollback() error
	Commit() error
}

type Addrs interface {
	database.DB
	GetFilter() *sdk.AddrFilter
	Put(addr *common.Uint168) error
	GetAll() []*common.Uint168
}

type Txs interface {
	database.DB
	Put(tx *util.Tx) error
	Get(txId *common.Uint256) (*util.Tx, error)
	GetAll() ([]*util.Tx, error)
	GetIds(height uint32) ([]*common.Uint256, error)
	PutForkTxs(txs []*util.Tx, hash *common.Uint256) error
	GetForkTxs(hash *common.Uint256) ([]*util.Tx, error)
	Del(txId *common.Uint256) error
	Batch() TxsBatch
}

type TxsBatch interface {
	batch
	Put(tx *util.Tx) error
	Del(txId *common.Uint256) error
	DelAll(height uint32) error
}

type Ops interface {
	database.DB
	Put(*util.OutPoint, common.Uint168) error
	HaveOp(*util.OutPoint) *common.Uint168
	GetAll() ([]*util.OutPoint, error)
	Batch() OpsBatch
}

type OpsBatch interface {
	batch
	Put(*util.OutPoint, common.Uint168) error
	Del(*util.OutPoint) error
}

type Que interface {
	database.DB

	// Put a queue item to database
	Put(item *QueItem) error

	// Get all items in queue
	GetAll() ([]*QueItem, error)

	// Delete confirmed item in queue
	Del(notifyId, txHash *common.Uint256) error

	// Batch returns a queue batch instance.
	Batch() QueBatch
}

type QueBatch interface {
	batch

	// Put a queue item to database
	Put(item *QueItem) error

	// Delete confirmed item in queue
	Del(notifyId, txHash *common.Uint256) error

	// Delete all items on the given height.
	DelAll(height uint32) error
}

type QueItem struct {
	NotifyId   common.Uint256
	TxId       common.Uint256
	Height     uint32
	LastNotify time.Time
}
