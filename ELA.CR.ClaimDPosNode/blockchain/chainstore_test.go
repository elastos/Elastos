// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var testChainStore *ChainStore
var sidechainTxHash common.Uint256

func TestChainStoreInit(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)
	temp, err := NewChainStore(test.DataPath, config.DefaultParams.GenesisBlock)
	assert.NoError(t, err)
	testChainStore = temp.(*ChainStore)
	testChainStore.NewBatch()
	if err != nil {
		t.Error("Create chainstore failed")
	}

	// Assume the sidechain Tx hash
	txHashStr := "39fc8ba05b0064381e51afed65b4cf91bb8db60efebc38242e965d1b1fed0701"
	txHashBytes, _ := common.HexStringToBytes(txHashStr)
	txHash, _ := common.Uint256FromBytes(txHashBytes)
	sidechainTxHash = *txHash
}

func TestChainStore_PersisSidechainTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx should not exist in DB.
	_, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should not exist in DB")
	}

	// 2. Run PersistSidechainTx
	testChainStore.PersistSidechainTx(sidechainTxHash)

	// Need batch commit here because PersistSidechainTx use BatchPut
	testChainStore.BatchCommit()

	// 3. Verify PersistSidechainTx
	_, err = testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
}

func TestChainStore_RollbackSidechainTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx hash should exist in DB.
	_, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}

	// 2. Run Rollback
	err = testChainStore.rollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	// Need batch commit here because rollbackSidechainTx use BatchDelete
	testChainStore.BatchCommit()

	// 3. Verify rollbackSidechainTx
	_, err = testChainStore.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should been deleted")
	}
}

func TestChainStore_IsSidechainTxHashDuplicate(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx should not exist in DB.
	_, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should not exist in DB")
	}

	// 2. Persist the sidechain Tx hash
	testChainStore.PersistSidechainTx(sidechainTxHash)

	// Need batch commit here because PersistSidechainTx use BatchPut
	testChainStore.BatchCommit()

	// 3. Verify PersistSidechainTx
	_, err = testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}

	// 4. Run IsSidechainTxHashDuplicate
	isDuplicate := testChainStore.isSidechainTxHashDuplicate(sidechainTxHash)
	if !isDuplicate {
		t.Error("Sidechain Tx hash should be checked to be duplicated")
	}
}

func TestCheckAssetPrecision(t *testing.T) {
	tx := buildTx()
	// valid precision
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
		output.Value = 123456789876
	}
	err := checkAssetPrecision(tx)
	assert.NoError(t, err)

	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
		output.Value = 0
	}
	err = checkAssetPrecision(tx)
	assert.NoError(t, err)
}

func TestChainStoreDone(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	err := testChainStore.rollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	testChainStore.BatchCommit()
	testChainStore.Close()
}
