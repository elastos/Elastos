package LevelDBStore

import (
	. "github.com/elastos/Elastos.ELA/store"
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

func (self *LevelDB) Put(key []byte, value []byte) error {
	return self.db.Put(key, value, nil)
}

func (self *LevelDB) Get(key []byte) ([]byte, error) {
	dat, err := self.db.Get(key, nil)
	return dat, err
}

func (self *LevelDB) Delete(key []byte) error {
	return self.db.Delete(key, nil)
}

func (self *LevelDB) NewBatch() error {
	self.batch = new(leveldb.Batch)
	return nil
}

func (self *LevelDB) BatchPut(key []byte, value []byte) error {
	self.batch.Put(key, value)
	return nil
}

func (self *LevelDB) BatchDelete(key []byte) error {
	self.batch.Delete(key)
	return nil
}

func (self *LevelDB) BatchCommit() error {
	err := self.db.Write(self.batch, nil)
	if err != nil {
		return err
	}
	return nil
}

func (self *LevelDB) Close() error {
	err := self.db.Close()
	return err
}

func (self *LevelDB) NewIterator(prefix []byte) IIterator {

	iter := self.db.NewIterator(util.BytesPrefix(prefix), nil)

	return &Iterator{
		iter: iter,
	}
}
