package store

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/errors"
	"github.com/syndtr/goleveldb/leveldb/filter"
	"github.com/syndtr/goleveldb/leveldb/opt"
	"github.com/syndtr/goleveldb/leveldb/util"
)

// used to compute the size of bloom filter bits array .
// too small will lead to high false positive rate.
const BITSPERKEY = 10

// Ensure LevelDB implements Database interface.
var _ Database = (*LevelDB)(nil)

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

func (l *LevelDB) Put(key []byte, value []byte) error {
	return l.db.Put(key, value, nil)
}

func (l *LevelDB) Get(key []byte) ([]byte, error) {
	return l.db.Get(key, nil)
}

func (l *LevelDB) Delete(key []byte) error {
	return l.db.Delete(key, nil)
}

func (l *LevelDB) NewBatch() Batch {
	return &batch{
		db:    l.db,
		batch: new(leveldb.Batch),
	}
}

func (l *LevelDB) NewIterator(prefix []byte) blockchain.IIterator {
	return l.db.NewIterator(util.BytesPrefix(prefix), nil)
}

func (l *LevelDB) Close() error {
	return l.db.Close()
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
