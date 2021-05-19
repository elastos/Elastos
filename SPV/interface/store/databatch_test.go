package store

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestDataBatch_DelAll(t *testing.T) {
	db, err := NewDataStore("test")
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	batch := db.Batch()
	err = batch.DelAll(0)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	err = batch.Commit()
	if !assert.NoError(t, err) {
		t.FailNow()
	}
}
