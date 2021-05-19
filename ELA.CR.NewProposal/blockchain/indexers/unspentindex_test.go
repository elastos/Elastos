// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package indexers

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"
	_ "github.com/elastos/Elastos.ELA/database/ffldb"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/btcsuite/btcd/wire"
	"github.com/stretchr/testify/assert"
)

var (
	unspentIndexReferTx1    = common.Uint256{1}
	unspentIndexReferTx2    = common.Uint256{2}
	unspentIndexReferTx3    = common.Uint256{3}
	unspentIndexReferIndex1 = uint16(1)
	unspentIndexReferIndex2 = uint16(2)
	unspentIndexReferIndex3 = uint16(3)
	unspentIndexCoinbase    = &types.Transaction{
		TxType:  types.CoinBase,
		Payload: new(payload.CoinBase),
		Inputs:  nil,
		Outputs: []*types.Output{
			{
				Value: 10,
			},
			{
				Value: 20,
			},
		},
	}
	testUnspentIndexTx1 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  unspentIndexReferTx1,
					Index: unspentIndexReferIndex1,
				},
			},
		},
		Outputs: []*types.Output{
			{
				Value: 30,
			},
			{
				Value: 40,
			},
		},
	}
	testUnspentIndexTx2 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  unspentIndexReferTx2,
					Index: unspentIndexReferIndex2,
				},
			},
		},
		Outputs: []*types.Output{
			{
				Value: 50,
			},
			{
				Value: 60,
			},
		},
	}
	testUnspentIndexTx3 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  unspentIndexReferTx3,
					Index: unspentIndexReferIndex3,
				},
			},
		},
		Outputs: []*types.Output{
			{
				Value: 0,
			},
			{
				Value: 50,
			},
		},
	}
	unspentIndexBlock = &types.Block{
		Header: types.Header{},
		Transactions: []*types.Transaction{
			unspentIndexCoinbase,
			testUnspentIndexTx1,
			testUnspentIndexTx2,
			testUnspentIndexTx3,
		},
	}
	testUnspentIndexTx4 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  testUnspentIndexTx3.Hash(),
					Index: 0,
				},
			},
			{
				Previous: types.OutPoint{
					TxID:  testUnspentIndexTx3.Hash(),
					Index: 1,
				},
			},
		},
		Outputs: []*types.Output{
			{
				Value: 40,
			},
		},
	}
	testUnspentIndexTx5 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs:  []*types.Input{},
		Outputs: []*types.Output{},
	}
	unspentIndexCoinbase2 = &types.Transaction{
		TxType:  types.CoinBase,
		Payload: new(payload.CoinBase),
		Inputs:  nil,
		Outputs: []*types.Output{
			{
				Value: 30,
			},
			{
				Value: 40,
			},
		},
	}
	unspentIndexBlock2 = &types.Block{
		Header: types.Header{},
		Transactions: []*types.Transaction{
			unspentIndexCoinbase2,
			testUnspentIndexTx4,
			testUnspentIndexTx5,
		},
	}
	testUnspentIndex *UnspentIndex
	unspentIndexDB   database.DB
)

func TestUnspentIndexInit(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	var err error
	unspentIndexDB, err = LoadBlockDB(test.DataPath)
	assert.NoError(t, err)
	testUnspentIndex = NewUnspentIndex(unspentIndexDB, &config.DefaultParams)
	assert.NotEqual(t, nil, testUnspentIndex)
	assert.Equal(t, unspentIndexKey, testUnspentIndex.Key())
	assert.Equal(t, unspentIndexName, testUnspentIndex.Name())
	_ = unspentIndexDB.Update(func(dbTx database.Tx) error {
		err := testUnspentIndex.Create(dbTx)
		assert.NoError(t, err)

		// initialize test unspent
		err = dbPutUnspentIndexEntry(dbTx, &unspentIndexReferTx1, []uint16{unspentIndexReferIndex1})
		assert.NoError(t, err)
		err = dbPutUnspentIndexEntry(dbTx, &unspentIndexReferTx2, []uint16{unspentIndexReferIndex2})
		assert.NoError(t, err)
		err = dbPutUnspentIndexEntry(dbTx, &unspentIndexReferTx3, []uint16{unspentIndexReferIndex3})
		assert.NoError(t, err)
		//testUnspentIndex.txCache.setTxn(1, &types.Transaction{
		//	LockTime: 10,
		//})
		//testUnspentIndex.txCache.setTxn(1, &types.Transaction{
		//	LockTime: 20,
		//})

		// check the initialization
		indexes, err := dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx1)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{unspentIndexReferIndex1}, indexes)
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx2)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{unspentIndexReferIndex2}, indexes)
		return nil
	})
}

func TestUnspentIndex_ConnectBlock(t *testing.T) {
	_ = unspentIndexDB.Update(func(dbTx database.Tx) error {
		err := testUnspentIndex.ConnectBlock(dbTx, unspentIndexBlock)
		assert.NoError(t, err)
		err = testUnspentIndex.ConnectBlock(dbTx, unspentIndexBlock2)
		assert.NoError(t, err)

		// the unspent txn should be cached
		assert.Equal(t, &TxInfo{
			txn:         unspentIndexCoinbase,
			blockHeight: unspentIndexBlock.Height,
		}, testUnspentIndex.txCache.getTxn(unspentIndexCoinbase.Hash()))
		assert.Equal(t, &TxInfo{
			txn:         testUnspentIndexTx1,
			blockHeight: unspentIndexBlock.Height,
		}, testUnspentIndex.txCache.getTxn(testUnspentIndexTx1.Hash()))
		assert.Equal(t, &TxInfo{
			txn:         testUnspentIndexTx2,
			blockHeight: unspentIndexBlock.Height,
		}, testUnspentIndex.txCache.getTxn(testUnspentIndexTx2.Hash()))

		// the spent txn should be removed
		assert.Equal(t, (*TxInfo)(nil), testUnspentIndex.txCache.getTxn(unspentIndexReferTx1))
		assert.Equal(t, (*TxInfo)(nil), testUnspentIndex.txCache.getTxn(unspentIndexReferTx2))

		// input items should be removed from db
		indexes, err := dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx1)
		assert.NoError(t, err)
		assert.Equal(t, []uint16(nil), indexes)
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx2)
		assert.NoError(t, err)
		assert.Equal(t, []uint16(nil), indexes)

		// output items should be added in db
		coinbaseHash := unspentIndexCoinbase.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &coinbaseHash)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{0, 1}, indexes)
		txHash1 := testUnspentIndexTx1.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &txHash1)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{0, 1}, indexes)
		txHash2 := testUnspentIndexTx1.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &txHash2)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{0, 1}, indexes)

		return nil
	})
}

func TestUnspentIndex_DisconnectBlock(t *testing.T) {
	_ = unspentIndexDB.Update(func(dbTx database.Tx) error {
		err := testUnspentIndex.DisconnectBlock(dbTx, unspentIndexBlock2)
		assert.NoError(t, err)

		err = testUnspentIndex.DisconnectBlock(dbTx, unspentIndexBlock)
		assert.NoError(t, err)

		// the spent txn should be removed
		assert.Equal(t, (*TxInfo)(nil), testUnspentIndex.txCache.getTxn(unspentIndexReferTx1))
		assert.Equal(t, (*TxInfo)(nil), testUnspentIndex.txCache.getTxn(unspentIndexReferTx2))

		// the refer txn should be cached
		//_, ok1 := testUnspentIndex.txns[unspentIndexReferTx1]
		//assert.True(t, ok1)
		//_, ok2 := testUnspentIndex.txns[unspentIndexReferTx2]
		//assert.True(t, ok2)

		// input items should be added in db
		indexes, err := dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx1)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{unspentIndexReferIndex1}, indexes)
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &unspentIndexReferTx2)
		assert.NoError(t, err)
		assert.Equal(t, []uint16{unspentIndexReferIndex2}, indexes)

		// output items should be removed from db
		coinbaseHash := unspentIndexCoinbase.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &coinbaseHash)
		assert.NoError(t, err)
		assert.Equal(t, []uint16(nil), indexes)
		txHash1 := testUnspentIndexTx1.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &txHash1)
		assert.NoError(t, err)
		assert.Equal(t, []uint16(nil), indexes)
		txHash2 := testUnspentIndexTx1.Hash()
		indexes, err = dbFetchUnspentIndexEntry(dbTx, &txHash2)
		assert.NoError(t, err)
		assert.Equal(t, []uint16(nil), indexes)

		return nil
	})
}

const (
	// blockDbNamePrefix is the prefix for the block database name.  The
	// database type is appended to this value to form the full block
	// database name.
	blockDbNamePrefix = "blocks"
)

// dbPath returns the path to the block database given a database type.
func blockDbPath(dataPath, dbType string) string {
	// The database name is based on the database type.
	dbName := blockDbNamePrefix + "_" + dbType
	if dbType == "sqlite" {
		dbName = dbName + ".db"
	}
	dbPath := filepath.Join(dataPath, dbName)
	return dbPath
}

// loadBlockDB loads (or creates when needed) the block database taking into
// account the selected database backend and returns a handle to it.  It also
// contains additional logic such warning the user if there are multiple
// databases which consume space on the file system and ensuring the regression
// test database is clean when in regression test mode.
func LoadBlockDB(dataPath string) (database.DB, error) {
	// The memdb backend does not have a file path associated with it, so
	// handle it uniquely.  We also don't want to worry about the multiple
	// database type warnings when running with the memory database.

	// The database name is based on the database type.
	dbType := "ffldb"
	dbPath := blockDbPath(dataPath, dbType)

	log.Infof("Loading block database from '%s'", dbPath)
	db, err := database.Open(dbType, dbPath, wire.MainNet)
	if err != nil {
		// Return the error if it's not because the database doesn't
		// exist.
		if dbErr, ok := err.(database.Error); !ok || dbErr.ErrorCode !=
			database.ErrDbDoesNotExist {

			return nil, err
		}

		// Create the db if it does not exist.
		err = os.MkdirAll(dataPath, 0700)
		if err != nil {
			return nil, err
		}
		db, err = database.Create(dbType, dbPath, wire.MainNet)
		if err != nil {
			return nil, err
		}
	}

	log.Info("Block database loaded")
	return db, nil
}

func TestUnspentIndexEnd(t *testing.T) {
	_ = unspentIndexDB.Update(func(dbTx database.Tx) error {
		meta := dbTx.Metadata()
		err := meta.DeleteBucket(unspentIndexKey)
		assert.NoError(t, err)
		return nil
	})
	unspentIndexDB.Close()
}
