package spvwallet

import (
	"crypto/rand"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"testing"
)

func TestTxIdCache(t *testing.T) {
	var cap = 100
	cache := NewTxIdCache(cap)

	var firstId common.Uint256
	var lastId common.Uint256
	for i := 0; i <= cap; i++ {
		var txId common.Uint256
		rand.Read(txId[:])

		if i == 0 {
			firstId = txId
		}
		ok := cache.Add(txId, uint32(i))
		if !ok {
			t.Errorf("Add txId failed at height %d", i)
		}

		t.Logf("TxId added %s height %d", txId.String(), i)

		if len(cache.txIds) > cap {
			t.Errorf("Cache overflow to size %d", len(cache.txIds))
		}

		t.Logf("Cache index %d size %d", cache.index, len(cache.txIds))

		lastId = txId
	}

	_, ok := cache.Get(firstId)
	if !ok {
		t.Errorf("Oldest txId not removed from cache")
	}

	height, ok := cache.Get(lastId)
	if ok {
		t.Errorf("Duplicated txId added %s", lastId.String())
	}

	t.Logf("Find cached txId at height %d", height)
}
