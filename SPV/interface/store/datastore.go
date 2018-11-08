package store

import (
	"github.com/boltdb/bolt"
	"path/filepath"
	"sync"
)

// Ensure dataStore implement DataStore interface.
var _ DataStore = (*dataStore)(nil)

type dataStore struct {
	*sync.RWMutex
	*bolt.DB
	addrs *addrs
	txs   *txs
	ops   *ops
	que   *que
}

func NewDataStore(dataDir string) (*dataStore, error) {
	db, err := bolt.Open(filepath.Join(dataDir, "data_store.bin"), 0644,
		&bolt.Options{InitialMmapSize: 5000000})
	if err != nil {
		return nil, err
	}
	store := new(dataStore)
	store.RWMutex = new(sync.RWMutex)
	store.DB = db

	store.addrs, err = NewAddrs(db)
	if err != nil {
		return nil, err
	}

	store.txs, err = NewTxs(db)
	if err != nil {
		return nil, err
	}

	store.ops, err = NewOps(db)
	if err != nil {
		return nil, err
	}

	store.que, err = NewQue(dataDir)
	if err != nil {
		return nil, err
	}

	return store, nil
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
	boltTx, err := d.DB.Begin(true)
	if err != nil {
		panic(err)
	}

	sqlTx, err := d.que.Begin()
	if err != nil {
		panic(err)
	}

	return &dataBatch{
		boltTx: boltTx,
		sqlTx:  sqlTx,
	}
}

func (d *dataStore) Clear() error {
	d.Lock()
	defer d.Unlock()

	return d.Update(func(tx *bolt.Tx) error {
		tx.DeleteBucket(BKTAddrs)
		tx.DeleteBucket(BKTTxs)
		tx.DeleteBucket(BKTHeightTxs)
		tx.DeleteBucket(BKTOps)
		return nil
	})
}

// Close db
func (d *dataStore) Close() error {
	d.addrs.Close()
	d.txs.Close()
	d.ops.Close()

	d.Lock()
	defer d.Unlock()

	d.que.Close()
	return d.DB.Close()
}
