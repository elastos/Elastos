// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package indexers

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var (
	tx3Hash = common.Uint256{1, 2, 3}
	tx4     = &types.Transaction{
		TxType:         types.WithdrawFromSideChain,
		PayloadVersion: 0,
		Payload: &payload.WithdrawFromSideChain{
			SideChainTransactionHashes: []common.Uint256{
				tx3Hash,
			},
		},
		Inputs:  []*types.Input{},
		Outputs: []*types.Output{},
	}

	testTx3IndexBlock = &types.Block{
		Header: types.Header{},
		Transactions: []*types.Transaction{
			tx4,
		},
	}

	testTx3Index *Tx3Index
	tx3IndexDB   database.DB
)

func TestTx3IndexInit(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	var err error
	tx3IndexDB, err = LoadBlockDB(test.DataPath)
	assert.NoError(t, err)

	testTx3Index = NewTx3Index(tx3IndexDB)
	assert.Equal(t, tx3IndexName, testTx3Index.Name())
	assert.Equal(t, tx3IndexKey, testTx3Index.Key())
	assert.NoError(t, testTx3Index.Init())

	_ = tx3IndexDB.Update(func(dbTx database.Tx) error {
		err := testTx3Index.Create(dbTx)
		assert.NoError(t, err)
		return err
	})
}

func TestTx3Index_ConnectBlock(t *testing.T) {
	_ = tx3IndexDB.Update(func(dbTx database.Tx) error {
		// tx3 should not in db
		assert.Equal(t, false, dbFetchTx3IndexEntry(dbTx, &tx3Hash))

		// connect the block
		err := testTx3Index.ConnectBlock(dbTx, testTx3IndexBlock)
		assert.NoError(t, err)

		// tx3 should be stored in db
		assert.Equal(t, true, dbFetchTx3IndexEntry(dbTx, &tx3Hash))

		return err
	})
}

func TestTx3Index_Disconnect(t *testing.T) {
	_ = tx3IndexDB.Update(func(dbTx database.Tx) error {
		// tx3 should be stored in db
		assert.Equal(t, true, dbFetchTx3IndexEntry(dbTx, &tx3Hash))

		// disconnect the block
		err := testTx3Index.DisconnectBlock(dbTx, testTx3IndexBlock)
		assert.NoError(t, err)

		// tx3 should be removed from db
		assert.Equal(t, false, dbFetchTx3IndexEntry(dbTx, &tx3Hash))

		return nil
	})
}

func TestTx3IndexEnd(t *testing.T) {
	_ = tx3IndexDB.Update(func(dbTx database.Tx) error {
		meta := dbTx.Metadata()
		err := meta.DeleteBucket(tx3IndexKey)
		assert.NoError(t, err)
		return nil
	})
	tx3IndexDB.Close()
}
