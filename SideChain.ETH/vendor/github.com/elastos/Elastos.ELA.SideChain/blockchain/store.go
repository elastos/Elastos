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

type IStore interface {
	Put(key []byte, value []byte) error
	Get(key []byte) ([]byte, error)
	Delete(key []byte) error
	NewBatch()
	BatchPut(key []byte, value []byte)
	BatchDelete(key []byte)
	BatchCommit() error
	Close() error
	NewIterator(prefix []byte) IIterator
}
