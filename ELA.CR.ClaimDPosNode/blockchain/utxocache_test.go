// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"fmt"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var (
	testUTXOCacheDB *ChainStore
	utxoCache       *UTXOCache

	// refer tx hash: 160da301e49617c037ae9b630919af52b8ac458202cd64558af7e0dcc753e307
	referTx = &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.TransferAsset,
		PayloadVersion: 0,
		Payload:        &payload.TransferAsset{},
		Attributes:     []*types.Attribute{},
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					Index: 0,
					TxID:  common.EmptyHash,
				},
				Sequence: 0,
			},
		},
		Outputs: []*types.Output{
			{
				Value:   100,
				Type:    types.OTVote,
				Payload: &outputpayload.VoteOutput{},
			},
		},
		LockTime: 5,
	}

	spendTx = &types.Transaction{
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					Index: 0,
					TxID:  referTx.Hash(),
				},
				Sequence: 0,
			},
		},
	}
)

func TestUTXOCache_Init(t *testing.T) {
	db, err := NewLevelDB(filepath.Join(test.DataPath, "test_utxo_cache_chain"))
	assert.NoError(t, err)
	testUTXOCacheDB = &ChainStore{
		IStore:           db,
		blockHashesCache: make([]common.Uint256, 0, BlocksCacheSize),
		blocksCache:      make(map[common.Uint256]*types.Block),
	}
	testUTXOCacheDB.NewBatch()

	fmt.Println("refer tx hash:", referTx.Hash().String())
	err = testUTXOCacheDB.persistTransaction(referTx, 0)
	assert.NoError(t, err)

	testUTXOCacheDB.BatchCommit()
}

func TestUTXOCache_GetTxReferenceInfo(t *testing.T) {
	utxoCache = NewUTXOCache(testUTXOCacheDB)

	// get tx reference form db and cache it first time.
	reference, err := utxoCache.GetTxReferenceInfo(spendTx)
	assert.NoError(t, err)
	for input, outputInfo := range reference {
		assert.Equal(t, referTx.Hash(), input.Previous.TxID)
		assert.Equal(t, uint16(0), input.Previous.Index)
		assert.Equal(t, uint32(0), input.Sequence)

		assert.Equal(t, common.Fixed64(100), outputInfo.output.Value)
		assert.Equal(t, types.OTVote, outputInfo.output.Type)
		assert.Equal(t, 1, outputInfo.inputsCount)
		assert.Equal(t, uint32(5), outputInfo.locktime)
		assert.Equal(t, types.TransferAsset, outputInfo.txtype)
	}

	// ensure above reference have been cached.
	err = deleteTestDBTx(referTx)
	assert.NoError(t, err)
	_, _, err = testUTXOCacheDB.GetTransaction(referTx.Hash())
	assert.Equal(t, "leveldb: not found", err.Error())

	reference, err = utxoCache.GetTxReferenceInfo(spendTx)
	assert.NoError(t, err)
	for input, outputInfo := range reference {
		assert.Equal(t, referTx.Hash(), input.Previous.TxID)
		assert.Equal(t, uint16(0), input.Previous.Index)
		assert.Equal(t, uint32(0), input.Sequence)

		assert.Equal(t, common.Fixed64(100), outputInfo.output.Value)
		assert.Equal(t, types.OTVote, outputInfo.output.Type)
		assert.Equal(t, 1, outputInfo.inputsCount)
		assert.Equal(t, uint32(5), outputInfo.locktime)
		assert.Equal(t, types.TransferAsset, outputInfo.txtype)
	}
}

func TestUTXOCache_CleanSpentUTXOs(t *testing.T) {
	block := &types.Block{
		Header: types.Header{},
		Transactions: []*types.Transaction{
			{
				Version: types.TxVersion09,
				TxType:  types.CoinBase,
			},
			spendTx,
		},
	}
	utxoCache.CleanSpentUTXOs(block)
	_, err := utxoCache.GetTxReferenceInfo(spendTx)
	assert.Equal(t, "GetTxReferenceInfo failed, previous transaction not found", err.Error())
}

func deleteTestDBTx(tx *types.Transaction) error {
	key := new(bytes.Buffer)
	key.WriteByte(byte(DATATransaction))
	hash := referTx.Hash()
	err := hash.Serialize(key)
	if err != nil {
		return err
	}
	return testUTXOCacheDB.Delete(key.Bytes())
}
