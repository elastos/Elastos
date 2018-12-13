package store

import (
	"crypto/rand"
	"testing"

	"github.com/elastos/Elastos.ELA.Utility/common"

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

	notifyIDs := make([]common.Uint256, 100)
	txHashes := make([]common.Uint256, 100)
	for i := range notifyIDs {
		rand.Read(notifyIDs[i][:])
	}
	for i := range txHashes {
		rand.Read(txHashes[i][:])
	}

	idIndex := make(map[common.Uint256]common.Uint256)
	for i, notifyID := range notifyIDs {
		que.Put(&QueItem{NotifyId: notifyID, TxId: txHashes[i]})
		idIndex[notifyIDs[i]] = txHashes[i]
	}

	items, err := que.GetAll()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	if !assert.Equal(t, 100, len(items)) {
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

	height0 := make([]byte, 4)
	for i, notifyID := range notifyIDs {
		value := append(notifyID[:], txHashes[i][:]...)
		height, err := que.db.Get(toKey(BKTQue, value...), nil)
		if !assert.NoError(t, err) {
			t.FailNow()
		}
		if !assert.Equal(t, height0, height) {
			t.FailNow()
		}
	}

	for i, notifyID := range notifyIDs {
		que.Del(&notifyID, &txHashes[i])
		if i+1 >= 50 {
			break
		}
	}

	for i, notifyID := range notifyIDs {
		value := append(notifyID[:], txHashes[i][:]...)
		_, err := que.db.Get(toKey(BKTQue, value...), nil)
		if i < 50 {
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
	if !assert.Equal(t, 50, len(items)) {
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
	if !assert.Equal(t, 100, len(items)) {
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
		if i+1 >= 50 {
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
		if i < 50 {
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
	for i := 50; i < 100; i++ {
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
