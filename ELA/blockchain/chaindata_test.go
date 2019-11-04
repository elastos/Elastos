// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var (
	testChainDataReferTx1    = common.Uint256{1}
	testChainDataReferTx2    = common.Uint256{2}
	testChainDataReferIndex1 = uint16(1)
	testChainDataReferIndex2 = uint16(2)
	testChainDataKey1        = new(bytes.Buffer)
	testChainDataKey2        = new(bytes.Buffer)
	testChainDataCoinbase    = &types.Transaction{
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
	testChainDataTx1 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  testChainDataReferTx1,
					Index: testChainDataReferIndex1,
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
	testChainDataTx2 = &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: new(payload.TransferAsset),
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  testChainDataReferTx2,
					Index: testChainDataReferIndex2,
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
	testChainDataBlock = &types.Block{
		Header: types.Header{},
		Transactions: []*types.Transaction{
			testChainDataCoinbase,
			testChainDataTx1,
			testChainDataTx2,
		},
	}

	testChainDataStore *ChainStore
)

func TestChainDataInit(t *testing.T) {
	store, err := NewChainStore(test.DataPath, config.DefaultParams.GenesisBlock)
	assert.NoError(t, err)
	testChainDataStore = store.(*ChainStore)
	testChainDataStore.NewBatch()

	// initialize unspent
	testChainDataKey1.WriteByte(byte(IXUnspent))
	err = testChainDataReferTx1.Serialize(testChainDataKey1)
	assert.NoError(t, err)
	testChainDataStore.BatchPut(testChainDataKey1.Bytes(), ToByteArray([]uint16{testChainDataReferIndex1}))

	testChainDataKey2.WriteByte(byte(IXUnspent))
	err = testChainDataReferTx2.Serialize(testChainDataKey2)
	assert.NoError(t, err)
	testChainDataStore.BatchPut(testChainDataKey2.Bytes(), ToByteArray([]uint16{testChainDataReferIndex2}))

	err = testChainDataStore.BatchCommit()
	assert.NoError(t, err)
}

func TestChainStore_PersistUnspent(t *testing.T) {
	err := testChainDataStore.persistUnspend(testChainDataBlock)
	assert.NoError(t, err)
	err = testChainDataStore.BatchCommit()
	assert.NoError(t, err)

	// input refer should be removed from unspent array
	_, err = testChainDataStore.Get(testChainDataKey1.Bytes())
	assert.Equal(t, "leveldb: not found", err.Error())
	_, err = testChainDataStore.Get(testChainDataKey2.Bytes())
	assert.Equal(t, "leveldb: not found", err.Error())

	// output should be added in unspent array
	unspentCoinbase, err := getUnspent(testChainDataCoinbase.Hash())
	assert.NoError(t, err)
	assert.Equal(t, []uint16{0, 1}, unspentCoinbase)

	unspent1, err := getUnspent(testChainDataTx1.Hash())
	assert.NoError(t, err)
	assert.Equal(t, []uint16{0, 1}, unspent1)

	unspent2, err := getUnspent(testChainDataTx2.Hash())
	assert.NoError(t, err)
	assert.Equal(t, []uint16{0, 1}, unspent2)
}

func Test_ChainData_RollbackUnspent(t *testing.T) {
	err := testChainDataStore.RollbackUnspend(testChainDataBlock)
	assert.NoError(t, err)
	err = testChainDataStore.BatchCommit()
	assert.NoError(t, err)

	// input refer should be added in unspent array
	input1, err := testChainDataStore.Get(testChainDataKey1.Bytes())
	assert.NoError(t, err)
	assert.Equal(t, ToByteArray([]uint16{testChainDataReferIndex1}), input1)
	input2, err := testChainDataStore.Get(testChainDataKey2.Bytes())
	assert.NoError(t, err)
	assert.Equal(t, ToByteArray([]uint16{testChainDataReferIndex2}), input2)

	// output should be removed from unspent array
	_, err = getUnspent(testChainDataCoinbase.Hash())
	assert.Equal(t, "leveldb: not found", err.Error())

	_, err = getUnspent(testChainDataTx1.Hash())
	assert.Equal(t, "leveldb: not found", err.Error())

	_, err = getUnspent(testChainDataTx2.Hash())
	assert.Equal(t, "leveldb: not found", err.Error())
}

func getUnspent(txHash common.Uint256) ([]uint16, error) {
	key := new(bytes.Buffer)
	key.WriteByte(byte(IXUnspent))
	err := txHash.Serialize(key)
	if err != nil {
		return nil, err
	}
	data, err := testChainDataStore.Get(key.Bytes())
	if err != nil {
		return nil, err
	}

	return GetUint16Array(data)
}
