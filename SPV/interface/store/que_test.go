package store

import (
	"crypto/rand"
	"encoding/binary"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/stretchr/testify/assert"
	"github.com/syndtr/goleveldb/leveldb"
)

func TestQue(t *testing.T) {
	db, err := leveldb.OpenFile("test", nil)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	que := NewQue(db)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	defer que.Clear()

	times := 1000

	notifyIDs := make([]common.Uint256, times)
	txHashes := make([]common.Uint256, times)
	for i := range notifyIDs {
		rand.Read(notifyIDs[i][:])
	}
	for i := range txHashes {
		rand.Read(txHashes[i][:])
	}

	idIndex := make(map[common.Uint256]common.Uint256)
	for i, notifyID := range notifyIDs {
		item := QueItem{NotifyId: notifyID, TxId: txHashes[i]}
		if i%2 == 1 {
			item.LastNotify = item.LastNotify.Add(time.Second)
		}
		que.Put(&item)
		idIndex[notifyIDs[i]] = txHashes[i]
	}

	items, err := que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, times, len(items)) {
		t.FailNow()
	}
	for _, item := range items {
		txID, ok := idIndex[item.NotifyId]
		if !assert.Equal(t, true, ok) {
			t.FailNow()
		}
		if !assert.Equal(t, item.TxId, txID) {
			t.FailNow()
		}
	}

	var defaultTime time.Time
	data0 := make([]byte, 12)
	data1 := make([]byte, 12)
	binary.BigEndian.PutUint64(data0[4:], uint64(defaultTime.Unix()))
	binary.BigEndian.PutUint64(data1[4:], uint64(defaultTime.Add(time.Second).Unix()))
	for i, notifyID := range notifyIDs {
		value := append(notifyID[:], txHashes[i][:]...)
		data, err := que.db.Get(toKey(BKTQue, value...), nil)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
		if i%2 == 0 {
			if !assert.Equal(t, data0, data) {
				t.FailNow()
			}
		} else {
			if !assert.Equal(t, data1, data) {
				t.FailNow()
			}
		}

	}

	for i, notifyID := range notifyIDs {
		err := que.Del(&notifyID, &txHashes[i])
		assert.NoError(t, err)
		if i+1 >= times/2 {
			break
		}
	}

	for i, notifyID := range notifyIDs {
		value := append(notifyID[:], txHashes[i][:]...)
		_, err := que.db.Get(toKey(BKTQue, value...), nil)
		if i < times/2 {
			if !assert.Error(t, err) {
				t.FailNow()
			}
			continue
		}

		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	items, err = que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, times/2, len(items)) {
		t.FailNow()
	}

	err = que.Clear()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	items, err = que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, 0, len(items)) {
		t.FailNow()
	}

	batch := que.Batch()
	for i, notifyID := range notifyIDs {
		batch.Put(&QueItem{NotifyId: notifyID, TxId: txHashes[i], Height: uint32(i)})
	}
	err = batch.Commit()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	items, err = que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, times, len(items)) {
		t.FailNow()
	}
	for _, item := range items {
		txID, ok := idIndex[item.NotifyId]
		if !assert.Equal(t, true, ok) {
			t.FailNow()
		}
		if !assert.Equal(t, item.TxId, txID) {
			t.FailNow()
		}
	}

	batch = que.Batch()
	for i, notifyID := range notifyIDs {
		batch.Del(&notifyID, &txHashes[i])
		if i+1 >= times/2 {
			break
		}
	}
	err = batch.Commit()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	for i, notifyID := range notifyIDs {
		value := append(notifyID[:], txHashes[i][:]...)
		_, err := que.db.Get(toKey(BKTQue, value...), nil)
		if i < times/2 {
			if !assert.Error(t, err) {
				t.FailNow()
			}
			continue
		}

		if !assert.NoError(t, err) {
			t.FailNow()
		}
	}

	batch = que.Batch()
	for i := times / 2; i < times; i++ {
		batch.DelAll(uint32(i))
	}
	err = batch.Commit()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	items, err = que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, 0, len(items)) {
		t.FailNow()
	}
}
