package store

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
	dbutil "github.com/syndtr/goleveldb/leveldb/util"
)

var (
	BKTOps = []byte("O")
)

// Ensure ops implement Ops interface.
var _ Ops = (*ops)(nil)

type ops struct {
	sync.RWMutex
	db *leveldb.DB
}

func NewOps(db *leveldb.DB) *ops {
	return &ops{db: db}
}

func (o *ops) Put(op *util.OutPoint, addr common.Uint168) error {
	o.Lock()
	defer o.Unlock()
	return o.db.Put(toKey(BKTOps, op.Bytes()...), addr.Bytes(), nil)
}

func (o *ops) HaveOp(op *util.OutPoint) (addr *common.Uint168) {
	o.RLock()
	defer o.RUnlock()

	addrBytes, err := o.db.Get(toKey(BKTOps, op.Bytes()...), nil)
	if err != nil {
		return nil
	}
	if addr, err = common.Uint168FromBytes(addrBytes); err != nil {
		return nil
	}
	return addr
}

func (o *ops) GetAll() (ops []*util.OutPoint, err error) {
	o.RLock()
	defer o.RUnlock()

	it := o.db.NewIterator(dbutil.BytesPrefix(BKTOps), nil)
	defer it.Release()
	for it.Next() {
		op, err := util.OutPointFromBytes(subKey(BKTOps, it.Key()))
		if err != nil {
			return nil, err
		}
		ops = append(ops, op)
	}

	return ops, it.Error()
}

func (o *ops) Batch() OpsBatch {
	return &opsBatch{DB: o.db, Batch: new(leveldb.Batch)}
}

func (o *ops) Clear() error {
	o.Lock()
	defer o.Unlock()
	it := o.db.NewIterator(dbutil.BytesPrefix(BKTOps), nil)
	defer it.Release()
	batch := new(leveldb.Batch)
	for it.Next() {
		batch.Delete(it.Key())
	}
	return o.db.Write(batch, nil)
}

func (o *ops) Close() error {
	o.Lock()
	return nil
}
