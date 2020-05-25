// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package utils

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestHistory_Commit(t *testing.T) {
	value := 0
	historyData := historyData{
		value:   &value,
		history: NewHistory(20),
	}

	historyData.appendToHistory(10, 10)
	historyData.history.Commit(10)
	assert.Equal(t, true, *historyData.value == 10)
	assert.Equal(t, true, historyData.history.height == 10)

	historyData.appendToHistory(11, 20)
	historyData.history.Commit(11)
	assert.Equal(t, true, *historyData.value == 20)
	assert.Equal(t, true, historyData.history.height == 11)

	historyData.appendToHistory(12, 30)
	historyData.history.Commit(12)
	assert.Equal(t, true, *historyData.value == 30)
	assert.Equal(t, true, historyData.history.height == 12)

	historyData.appendToHistory(13, 40)
	historyData.history.Commit(13)
	assert.Equal(t, true, *historyData.value == 40)
	assert.Equal(t, true, historyData.history.height == 13)

	historyData.appendToHistory(20, 50)
	historyData.history.Commit(20)
	assert.Equal(t, true, *historyData.value == 50)
	assert.Equal(t, true, historyData.history.height == 20)

	historyData.appendToHistory(30, 60)
	historyData.history.Commit(30)
	assert.Equal(t, true, *historyData.value == 60)
	assert.Equal(t, true, historyData.history.height == 30)

	historyData.appendToHistory(40, 70)
	historyData.history.Commit(40)
	assert.Equal(t, true, *historyData.value == 70)
	assert.Equal(t, true, historyData.history.height == 40)

	historyData.history.RollbackTo(39)
	assert.Equal(t, true, *historyData.value == 60)
	assert.Equal(t, true, historyData.history.height == 39)

	historyData.history.RollbackTo(38)
	assert.Equal(t, true, *historyData.value == 60)
	assert.Equal(t, true, historyData.history.height == 38)

	historyData.history.RollbackTo(37)
	assert.Equal(t, true, *historyData.value == 60)
	assert.Equal(t, true, historyData.history.height == 37)

	historyData.history.RollbackTo(30)
	assert.Equal(t, true, *historyData.value == 60)
	assert.Equal(t, true, historyData.history.height == 30)

	historyData.history.RollbackTo(20)
	assert.Equal(t, true, *historyData.value == 50)
	assert.Equal(t, true, historyData.history.height == 20)

	historyData.history.RollbackTo(13)
	assert.Equal(t, true, *historyData.value == 40)
	assert.Equal(t, true, historyData.history.height == 13)

	historyData.history.RollbackTo(12)
	assert.Equal(t, true, *historyData.value == 30)
	assert.Equal(t, true, historyData.history.height == 12)

	historyData.history.RollbackTo(11)
	assert.Equal(t, true, *historyData.value == 20)
	assert.Equal(t, true, historyData.history.height == 11)

	historyData.history.RollbackTo(10)
	assert.Equal(t, true, *historyData.value == 10)
	assert.Equal(t, true, historyData.history.height == 10)

	historyData.history.RollbackTo(1)
	assert.Equal(t, true, *historyData.value == 0)
	assert.Equal(t, true, historyData.history.height == 1)
}

type historyData struct {
	value   *int
	history *History
}

func (h *historyData) appendToHistory(height uint32, setValue int) {
	oriValue := *h.value
	h.history.Append(height, func() {
		h.value = &setValue
	}, func() {
		h.value = &oriValue
	})
}
