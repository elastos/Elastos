package db

import (
	"sync"

	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/boltdb/bolt"
)

var (
	BKTOps = []byte("Ops")
)

type Outpoints interface {
	Put(*core.OutPoint, common.Uint168) error
	IsExist(*core.OutPoint) *common.Uint168
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

func (t *OutpointStore) Put(op *core.OutPoint, addr common.Uint168) (err error) {
	t.Lock()
	defer t.Unlock()
	return t.Update(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTOps).Put(op.Bytes(), addr.Bytes())
	})
}

func (t *OutpointStore) IsExist(op *core.OutPoint) (addr *common.Uint168) {
	t.RLock()
	defer t.RUnlock()

	t.View(func(tx *bolt.Tx) error {
		addrBytes := tx.Bucket(BKTOps).Get(op.Bytes())
		var err error
		if addr, err = common.Uint168FromBytes(addrBytes); err != nil {
			return err
		}
		return nil
	})
	return addr
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
