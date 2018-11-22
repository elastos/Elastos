package store

import (
	"path/filepath"
	"sync"

	"github.com/syndtr/goleveldb/leveldb"
)

// Ensure dataStore implement DataStore interface.
var _ DataStore = (*dataStore)(nil)

type dataStore struct {
	sync.RWMutex
	db    *leveldb.DB
	addrs *addrs
	txs   *txs
	ops   *ops
	que   *que
}

func NewDataStore(dataDir string) (*dataStore, error) {
	db, err := leveldb.OpenFile(filepath.Join(dataDir, "DATA"), nil)
	if err != nil {
		return nil, err
	}
	if err != nil {
		return nil, err
	}

	addrs, err := NewAddrs(db)
	if err != nil {
		return nil, err
	}

	txs, err := NewTxs(db)
	if err != nil {
		return nil, err
	}

	ops, err := NewOps(db)
	if err != nil {
		return nil, err
	}

	que, err := NewQue(dataDir)
	if err != nil {
		return nil, err
	}

	return &dataStore{
		db:    db,
		addrs: addrs,
		txs:   txs,
		ops:   ops,
		que:   que,
	}, nil
}

func (d *dataStore) Addrs() Addrs {
	return d.addrs
}

func (d *dataStore) Txs() Txs {
	return d.txs
}

func (d *dataStore) Ops() Ops {
	return d.ops
}

func (d *dataStore) Que() Que {
	return d.que
}

func (d *dataStore) Batch() DataBatch {
	sqlTx, err := d.que.Begin()
	if err != nil {
		panic(err)
	}

	return &dataBatch{
		DB:d.db,
		Batch: new(leveldb.Batch),
		sqlTx:  sqlTx,
	}
}

func (d *dataStore) Clear() error {
	d.Lock()
	defer d.Unlock()

	d.que.Clear()

	it := d.db.NewIterator(nil, nil)
	batch := new(leveldb.Batch)
	for it.Next() {
		batch.Delete(it.Key())
	}
	it.Release()

	return d.db.Write(batch, nil)
}

// Close db
func (d *dataStore) Close() error {
	d.Lock()
	defer d.Unlock()

	d.addrs.Close()
	d.txs.Close()
	d.ops.Close()
	d.que.Close()
	return d.db.Close()
}
