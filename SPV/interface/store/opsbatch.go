package store

import (
	"github.com/boltdb/bolt"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

// Ensure opsBatch implement OpsBatch interface.
var _ OpsBatch = (*opsBatch)(nil)

type opsBatch struct {
	sync.Mutex
	*bolt.Tx
}

func (b *opsBatch) Put(op *core.OutPoint, addr common.Uint168) error {
	b.Lock()
	defer b.Unlock()
	return b.Tx.Bucket(BKTOps).Put(op.Bytes(), addr.Bytes())
}

func (b *opsBatch) Del(op *core.OutPoint) error {
	b.Lock()
	defer b.Unlock()
	return b.Tx.Bucket(BKTOps).Delete(op.Bytes())
}
