package store

import (
	"crypto/rand"
	"encoding/binary"
	math "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestTxIds(t *testing.T) {

	var data []byte

	var txIds []*common.Uint256
	for i := 0; i < 10; i++ {
		var txId common.Uint256
		rand.Read(txId[:])
		txIds = append(txIds, &txId)
	}

	if !assert.Equal(t, 10, len(txIds)) {
		t.FailNow()
	}

	data = putTxId(data, txIds[0])
	if !assert.Equal(t, 34, len(data)) {
		t.FailNow()
	}

	data = delTxId(data, txIds[0])
	if !assert.Equal(t, 2, len(data)) {
		t.FailNow()
	}

	for i, txId := range txIds {
		data = putTxId(data, txId)
		if !assert.Equal(t, i+1, len(getTxIds(data))) {
			t.FailNow()
		}
	}

	for txIds := getTxIds(data); len(txIds) > 0; txIds = getTxIds(data) {
		count := binary.BigEndian.Uint16(data[:2])
		txId := txIds[math.Intn(int(count))]
		data = delTxId(data, txId)
		if !assert.Equal(t, len(txIds)-1, len(getTxIds(data))) {
			t.FailNow()
		}
	}
}
