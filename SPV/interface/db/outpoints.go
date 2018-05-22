package db

import (
	"github.com/boltdb/bolt"
	"sync"

	"github.com/elastos/Elastos.ELA/core"
)

var (
	BKTOps = []byte("Ops")
)

type Outpoints interface {
	Put(*core.OutPoint) error
	IsExist(*core.OutPoint) bool
	GetAll() ([]*core.OutPoint, error)
}

type OutpointStore struct {
	*sync.RWMutex
	*bolt.DB
}

func NewOutpointDB(db *bolt.DB) (*OutpointStore, error) {
	store := new(OutpointStore)
	store.RWMutex = new(sync.RWMutex)
	store.DB = db

	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTOps)
		if err != nil {
			return err
		}
		return nil
	})

	return store, nil
}

func (t *OutpointStore) Put(op *core.OutPoint) (err error) {
	t.Lock()
	defer t.Unlock()
	return t.Update(func(tx *bolt.Tx) error {
		outpoint := op.Bytes()
		return tx.Bucket(BKTOps).Put(outpoint, outpoint)
	})
}

func (t *OutpointStore) IsExist(op *core.OutPoint) (exist bool) {
	t.RLock()
	defer t.RUnlock()
	t.View(func(tx *bolt.Tx) error {
		outpoint := tx.Bucket(BKTOps).Get(op.Bytes())
		if outpoint != nil {
			exist = true
		}
		return nil
	})
	return exist
}

func (t *OutpointStore) GetAll() (ops []*core.OutPoint, err error) {
	t.RLock()
	defer t.RUnlock()

	err = t.View(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTOps).ForEach(func(k, v []byte) error {
			op, err := core.OutPointFromBytes(v)
			if err != nil {
				return err
			}
			ops = append(ops, op)
			return nil
		})
	})

	return ops, err
}
