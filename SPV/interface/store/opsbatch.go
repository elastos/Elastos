package store

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure opsBatch implement OpsBatch interface.
var _ OpsBatch = (*opsBatch)(nil)

type opsBatch struct {
	sync.Mutex
	*bolt.Tx
}

func (b *opsBatch) Put(op *util.OutPoint, addr common.Uint168) error {
	b.Lock()
	defer b.Unlock()
	return b.Tx.Bucket(BKTOps).Put(op.Bytes(), addr.Bytes())
}

func (b *opsBatch) Del(op *util.OutPoint) error {
	b.Lock()
	defer b.Unlock()
	return b.Tx.Bucket(BKTOps).Delete(op.Bytes())
}
