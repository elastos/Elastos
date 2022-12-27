package database

type Iterator interface {
	Next() bool
	Prev() bool
	First() bool
	Last() bool
	Seek(key []byte) bool
	Key() []byte
	Value() []byte
	Release()
}

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
	NewIterator(prefix []byte) Iterator
	Close() error
}
