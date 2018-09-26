package blockchain

import (
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/errors"
	"github.com/syndtr/goleveldb/leveldb/filter"
	"github.com/syndtr/goleveldb/leveldb/opt"
	"github.com/syndtr/goleveldb/leveldb/util"
)

// used to compute the size of bloom filter bits array .
// too small will lead to high false positive rate.
const BITSPERKEY = 10

// Ensure LevelDB implements IStore interface.
var _ IStore = (*LevelDB)(nil)

type LevelDB struct {
	db *leveldb.DB // LevelDB instance
}

func NewLevelDB(file string) (*LevelDB, error) {
	// default Options
	o := opt.Options{
		NoSync: false,
		Filter: filter.NewBloomFilter(BITSPERKEY),
	}

	db, err := leveldb.OpenFile(file, &o)

	if _, corrupted := err.(*errors.ErrCorrupted); corrupted {
		db, err = leveldb.RecoverFile(file, nil)
	}

	if err != nil {
		return nil, err
	}

	return &LevelDB{
		db: db,
	}, nil
}

func (db *LevelDB) Put(key []byte, value []byte) error {
	return db.db.Put(key, value, nil)
}

func (db *LevelDB) Get(key []byte) ([]byte, error) {
	return db.db.Get(key, nil)
}

func (db *LevelDB) Delete(key []byte) error {
	return db.db.Delete(key, nil)
}

func (db *LevelDB) NewBatch() IBatch {
	return &batch{
		db:    db.db,
		batch: new(leveldb.Batch),
	}
}

func (db *LevelDB) NewIterator(prefix []byte) IIterator {
	iter := db.db.NewIterator(util.BytesPrefix(prefix), nil)
	return &Iterator{iter: iter}
}

func (db *LevelDB) Close() error {
	return db.db.Close()
}

type batch struct {
	db    *leveldb.DB // LevelDB instance
	batch *leveldb.Batch
}

func (b *batch) Put(key []byte, value []byte) error {
	b.batch.Put(key, value)
	return nil
}

func (b *batch) Delete(key []byte) error {
	b.batch.Delete(key)
	return nil
}

func (b *batch) Commit() error {
	return b.db.Write(b.batch, nil)
}

func (b *batch) Rollback() error {
	b.batch.Reset()
	return nil
}
