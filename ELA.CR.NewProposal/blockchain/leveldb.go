// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"github.com/elastos/Elastos.ELA/utils"

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
	if !utils.FileExisted(file) {
		return nil, nil
	}
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

func (ldb *LevelDB) Put(key []byte, value []byte) error {
	return ldb.db.Put(key, value, nil)
}

func (ldb *LevelDB) Get(key []byte) ([]byte, error) {
	return ldb.db.Get(key, nil)
}

func (ldb *LevelDB) Delete(key []byte) error {
	return ldb.db.Delete(key, nil)
}

func (ldb *LevelDB) NewBatch() {
	ldb.batch = new(leveldb.Batch)
}

func (ldb *LevelDB) BatchPut(key []byte, value []byte) {
	ldb.batch.Put(key, value)
}

func (ldb *LevelDB) BatchDelete(key []byte) {
	ldb.batch.Delete(key)
}

func (ldb *LevelDB) BatchCommit() error {
	return ldb.db.Write(ldb.batch, nil)
}

func (ldb *LevelDB) Close() error {
	return ldb.db.Close()
}

func (ldb *LevelDB) NewIterator(prefix []byte) IIterator {
	iter := ldb.db.NewIterator(util.BytesPrefix(prefix), nil)
	return &Iterator{iter: iter}
}
