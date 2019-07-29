package store

import (
	"encoding/binary"
	"sync"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/util"
)

// Ensure queBatch implement QueBatch interface.
var _ QueBatch = (*queBatch)(nil)

type queBatch struct {
	sync.Mutex
	*leveldb.DB
	*leveldb.Batch
}

// Put a queue item to database
func (b *queBatch) Put(item *QueItem) error {
	b.Lock()
	defer b.Unlock()

	var height [4]byte
	binary.BigEndian.PutUint32(height[:], item.Height)
	value := append(item.NotifyId[:], item.TxId[:]...)
	b.Batch.Put(toKey(BKTQue, value...), height[:])
	b.Batch.Put(toKey(BKTQueIdx, append(height[:], value...)...), empty)
	return nil
}

// Delete confirmed item in queue
func (b *queBatch) Del(notifyId, txHash *common.Uint256) error {
	b.Lock()
	defer b.Unlock()

	value := append(notifyId[:], txHash[:]...)
	height, err := b.DB.Get(toKey(BKTQue, value...), nil)
	if err != nil {
		return err
	}
	b.Batch.Delete(toKey(BKTQue, value...))
	b.Batch.Delete(toKey(BKTQueIdx, append(height[:], value...)...))
	return nil
}

// Delete all items on the given height.
func (b *queBatch) DelAll(height uint32) error {
	b.Lock()
	defer b.Unlock()

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], height)
	prefix := toKey(BKTQueIdx, key[:]...)
	it := b.DB.NewIterator(util.BytesPrefix(prefix), nil)
	for it.Next() {
		value := subKey(prefix, it.Key())
		b.Batch.Delete(toKey(BKTQue, value...))
		b.Batch.Delete(it.Key())
	}
	it.Release()
	return nil
}

func (b *queBatch) Rollback() error {
	b.Lock()
	defer b.Unlock()
	b.Batch.Reset()
	return nil
}

func (b *queBatch) Commit() error {
	b.Lock()
	defer b.Unlock()
	return b.Write(b.Batch, nil)
}
