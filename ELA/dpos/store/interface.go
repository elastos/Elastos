package store

import "github.com/elastos/Elastos.ELA/blockchain"

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
