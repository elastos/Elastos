package store

import (
	"sync"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

var (
	BKTOps = []byte("Ops")
)

// Ensure ops implement Ops interface.
var _ Ops = (*ops)(nil)

type ops struct {
	*sync.RWMutex
	*bolt.DB
}

func NewOps(db *bolt.DB) (*ops, error) {
	store := new(ops)
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

func (o *ops) Put(op *core.OutPoint, addr common.Uint168) (err error) {
	o.Lock()
	defer o.Unlock()
	return o.Update(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTOps).Put(op.Bytes(), addr.Bytes())
	})
}

func (o *ops) IsExist(op *core.OutPoint) (addr *common.Uint168) {
	o.RLock()
	defer o.RUnlock()

	o.View(func(tx *bolt.Tx) error {
		addrBytes := tx.Bucket(BKTOps).Get(op.Bytes())
		var err error
		if addr, err = common.Uint168FromBytes(addrBytes); err != nil {
			return err
		}
		return nil
	})
	return addr
}

func (o *ops) GetAll() (ops []*core.OutPoint, err error) {
	o.RLock()
	defer o.RUnlock()

	err = o.View(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTOps).ForEach(func(k, v []byte) error {
			op, err := core.OutPointFromBytes(k)
			if err != nil {
				return err
			}
			ops = append(ops, op)
			return nil
		})
	})

	return ops, err
}

func (o *ops) Batch() OpsBatch {
	tx, err := o.Begin(true)
	if err != nil {
		panic(err)
	}

	return &opsBatch{Tx: tx}
}

func (o *ops) Clear() error {
	o.Lock()
	defer o.Unlock()
	return o.DB.Update(func(tx *bolt.Tx) error {
		return tx.DeleteBucket(BKTOps)
	})
}

func (o *ops) Close() error {
	o.Lock()
	return nil
}
