package store

import (
	"bytes"
	"encoding/binary"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/util"
)

var (
	BKTQue    = []byte("Q")
	BKTQueIdx = []byte("U")
	empty     = make([]byte, 0)
)

// Ensure que implement Que interface.
var _ Que = (*que)(nil)

type que struct {
	sync.RWMutex
	db *leveldb.DB
}

func NewQue(db *leveldb.DB) *que {
	return &que{db: db}
}

// Put a queue item to database
func (q *que) Put(item *QueItem) error {
	q.Lock()
	defer q.Unlock()

	batch := new(leveldb.Batch)
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.BigEndian, item.Height)
	value := append(item.NotifyId[:], item.TxId[:]...)
	batch.Put(toKey(BKTQueIdx, append(buf.Bytes(), value...)...), empty)
	binary.Write(buf, binary.BigEndian, item.LastNotify.Unix())
	batch.Put(toKey(BKTQue, value...), buf.Bytes())
	return q.db.Write(batch, nil)
}

// Get all items in queue
func (q *que) GetAll() (items []*QueItem, err error) {
	q.RLock()
	defer q.RUnlock()

	it := q.db.NewIterator(util.BytesPrefix(BKTQue), nil)
	defer it.Release()
	for it.Next() {
		var item QueItem
		var lastNotify int64
		value := subKey(BKTQue, it.Key())
		copy(item.NotifyId[:], value[:32])
		copy(item.TxId[:], value[32:])
		buf := bytes.NewReader(it.Value())
		binary.Read(buf, binary.BigEndian, &item.Height)
		binary.Read(buf, binary.BigEndian, &lastNotify)
		item.LastNotify = time.Unix(lastNotify, 0)
		items = append(items, &item)
	}
	return items, nil
}

// Delete confirmed item in queue
func (q *que) Del(notifyId, txHash *common.Uint256) error {
	q.Lock()
	defer q.Unlock()

	value := append(notifyId[:], txHash[:]...)
	height, err := q.db.Get(toKey(BKTQue, value...), nil)
	if err != nil {
		return err
	}
	batch := new(leveldb.Batch)
	batch.Delete(toKey(BKTQue, value...))
	batch.Delete(toKey(BKTQueIdx, append(height[:], value...)...))
	return q.db.Write(batch, nil)
}

func (q *que) Batch() QueBatch {
	return &queBatch{DB: q.db, Batch: new(leveldb.Batch)}
}

func (q *que) Clear() error {
	q.Lock()
	defer q.Unlock()

	batch := new(leveldb.Batch)
	it := q.db.NewIterator(util.BytesPrefix(BKTQue), nil)
	for it.Next() {
		batch.Delete(it.Key())
	}
	it.Release()

	it = q.db.NewIterator(util.BytesPrefix(BKTQueIdx), nil)
	for it.Next() {
		batch.Delete(it.Key())
	}
	it.Release()

	return q.db.Write(batch, nil)
}

func (q *que) Close() error {
	q.Lock()
	return nil
}
