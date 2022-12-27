package store

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/dpos/state"
)

type Batch interface {
	Put(key []byte, value []byte) error
	Delete(key []byte) error
	Commit() error
	Rollback() error
}

type Database interface {
	Put(key []byte, value []byte) error
	Get(key []byte) ([]byte, error)
	Delete(key []byte) error
	NewBatch() Batch
	NewIterator(prefix []byte) blockchain.IIterator
	Close() error
}

type IDBOperator interface {
	Create(table *DBTable) error
	Insert(table *DBTable, fields []*Field) (uint64, error)
	Select(table *DBTable, inputFields []*Field) ([][]*Field, error)
	Update(table *DBTable, inputFields []*Field, updateFields []*Field) ([]uint64, error)
	SelectID(table *DBTable, inputFields []*Field) ([]uint64, error)
	Close() error
}

type DirectPeers struct {
	PublicKey []byte
	Address   string
	Sequence  uint32
}

type IEventRecord interface {
	StartEventRecord()
	AddProposalEvent(event interface{}) error
	UpdateProposalEvent(event interface{}) error
	AddVoteEvent(event interface{}) error
	AddViewEvent(event interface{}) error
	AddConsensusEvent(event interface{}) error
	UpdateConsensusEvent(event interface{}) error
}

// IDposStore provides func for dpos
type IDposStore interface {
	IDBOperator
	IEventRecord
	state.IArbitratorsRecord
}
