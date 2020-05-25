// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package config

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestGenesisBlock(t *testing.T) {
	block := GenesisBlock(&mainNetFoundation)
	assert.Equal(t, len(block.Transactions), 2)

	genesisHash := block.Hash().String()
	assert.Equal(t, "8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405", genesisHash)

	genesisHash = GenesisBlock(&testNetFoundation).Hash().String()
	assert.Equal(t, "b3314f465ea5556d570bcc473d59a0855b4405a25b1ea0c957c81b2920be1864", genesisHash)

	date := time.Date(2017, time.December, 22, 10,
		0, 0, 0, time.UTC).Unix()
	dateUnix := time.Unix(time.Date(2017, time.December, 22, 10,
		0, 0, 0, time.UTC).Unix(), 0).Unix()

	dateTime, err := time.Parse(time.RFC3339, "2017-12-22T10:00:00Z")
	assert.NoError(t, err)
	assert.Equal(t, date, dateUnix)
	assert.Equal(t, date, dateTime.Unix())
}

func TestFoundation(t *testing.T) {
	address, _ := mainNetFoundation.ToAddress()
	assert.Equal(t, "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta", address)

	address, _ = testNetFoundation.ToAddress()
	assert.Equal(t, "8ZNizBf4KhhPjeJRGpox6rPcHE5Np6tFx3", address)
}
