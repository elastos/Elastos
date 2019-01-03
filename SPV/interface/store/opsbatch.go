package store

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
)

// Ensure opsBatch implement OpsBatch interface.
var _ OpsBatch = (*opsBatch)(nil)

type opsBatch struct {
	sync.Mutex
	*leveldb.DB
	*leveldb.Batch
}

func (b *opsBatch) Put(op *util.OutPoint, addr common.Uint168) error {
	b.Lock()
	defer b.Unlock()
	b.Batch.Put(toKey(BKTOps, op.Bytes()...), addr[:])
	return nil
}

func (b *opsBatch) Del(op *util.OutPoint) error {
	b.Lock()
	defer b.Unlock()
	b.Batch.Delete(toKey(BKTOps, op.Bytes()...))
	return nil
}

func (b *opsBatch) Rollback() error {
	b.Batch.Reset()
	return nil
}

func (b *opsBatch) Commit() error {
	return b.DB.Write(b.Batch, nil)
}
