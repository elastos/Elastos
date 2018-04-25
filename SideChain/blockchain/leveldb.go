package blockchain

import (
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/errors"
	"github.com/syndtr/goleveldb/leveldb/filter"
	"github.com/syndtr/goleveldb/leveldb/opt"
	"github.com/syndtr/goleveldb/leveldb/util"
)

type LevelDB struct {
	db    *leveldb.DB // LevelDB instance
	batch *leveldb.Batch
}

// used to compute the size of bloom filter bits array .
// too small will lead to high false positive rate.
const BITSPERKEY = 10

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
		db:    db,
		batch: nil,
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

func (db *LevelDB) NewBatch() {
	db.batch = new(leveldb.Batch)
}

func (db *LevelDB) BatchPut(key []byte, value []byte) {
	db.batch.Put(key, value)
}

func (db *LevelDB) BatchDelete(key []byte) {
	db.batch.Delete(key)
}

func (db *LevelDB) BatchCommit() error {
	return db.db.Write(db.batch, nil)
}

func (db *LevelDB) Close() error {
	return db.db.Close()
}

func (db *LevelDB) NewIterator(prefix []byte) IIterator {
	iter := db.db.NewIterator(util.BytesPrefix(prefix), nil)
	return &Iterator{iter: iter}
}
