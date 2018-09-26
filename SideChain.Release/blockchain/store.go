package blockchain

type IIterator interface {
	Next() bool
	Prev() bool
	First() bool
	Last() bool
	Seek(key []byte) bool
	Key() []byte
	Value() []byte
	Release()
}

type IBatch interface {
	Put(key []byte, value []byte) error
	Delete(key []byte) error
	Commit() error
	Rollback() error
}

type IStore interface {
	Put(key []byte, value []byte) error
	Get(key []byte) ([]byte, error)
	Delete(key []byte) error
	NewBatch() IBatch
	NewIterator(prefix []byte) IIterator
	Close() error
}
