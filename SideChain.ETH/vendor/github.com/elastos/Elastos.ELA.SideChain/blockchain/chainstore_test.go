package blockchain

import (
	"container/list"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
)

var testChainStore *ChainStore
var mainchainTxHash common.Uint256

func newTestChainStore() (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	levelDB, err := database.NewLevelDB("Chain_UnitTest")
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		Database:           levelDB,
		headerIndex:        map[uint32]common.Uint256{},
		headerCache:        map[common.Uint256]*types.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	go store.taskHandler()
	store.NewBatch()

	return store, nil
}

func TestChainStoreInit(t *testing.T) {
	// Get new chainstore
	var err error
	testChainStore, err = newTestChainStore()
	if err != nil {
		t.Error("Create chainstore failed")
	}

	// Assume the mainchain Tx hash
	txHashStr := "39fc8ba05b0064381e51afed65b4cf91bb8db60efebc38242e965d1b1fed0701"
	txHashBytes, _ := common.HexStringToBytes(txHashStr)
	txHash, _ := common.Uint256FromBytes(txHashBytes)
	mainchainTxHash = *txHash
}

func TestChainStore_PersisMainchainTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The mainchain Tx should not exist in DB.
	_, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err == nil {
		t.Error("Found the mainchain Tx which should not exist in DB")
	}

	// 2. Run PersistMainchainTx
	batch := testChainStore.Database.NewBatch()
	testChainStore.PersistMainchainTx(batch, mainchainTxHash)

	// Need batch commit here because PersistMainchainTx use BatchPut
	batch.Commit()

	// 3. Verify PersistMainchainTx
	exist, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Not found the mainchain Tx")
	}
	if exist != ValueExist {
		t.Error("Mainchian Tx matched wrong value")
	}
}

func TestChainStore_RollbackMainchainTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The mainchain Tx hash should exist in DB.
	exist, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Not found the mainchain Tx")
	}
	if exist != ValueExist {
		t.Error("Mainchian Tx matched wrong value")
	}

	// 2. Run Rollback
	batch := testChainStore.Database.NewBatch()
	err = testChainStore.RollbackMainchainTx(batch, mainchainTxHash)
	if err != nil {
		t.Error("Rollback the mainchain Tx failed")
	}

	// Need batch commit here because RollbackMainchainTx use BatchDelete
	batch.Commit()

	// 3. Verify RollbackMainchainTx
	_, err = testChainStore.GetMainchainTx(mainchainTxHash)
	if err == nil {
		t.Error("Found the mainchain Tx which should been deleted")
	}
}

func TestChainStore_IsMainchainTxHashDuplicate(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The mainchain Tx should not exist in DB.
	_, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err == nil {
		t.Error("Found the mainchain Tx which should not exist in DB")
	}

	// 2. Persist the mainchain Tx hash
	batch := testChainStore.Database.NewBatch()
	testChainStore.PersistMainchainTx(batch, mainchainTxHash)

	// Need batch commit here because PersistMainchainTx use BatchPut
	batch.Commit()

	// 3. Verify PersistMainchainTx
	exist, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Not found the mainchain Tx")
	}
	if exist != ValueExist {
		t.Error("Mainchian Tx matched wrong value")
	}

	// 4. Run IsMainchainTxHashDuplicate
	isDuplicate := testChainStore.IsDuplicateMainchainTx(mainchainTxHash)
	if !isDuplicate {
		t.Error("Mainchain Tx hash should be checked to be duplicated")
	}
}

func TestChainStoreDone(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
		return
	}

	batch := testChainStore.Database.NewBatch()
	err := testChainStore.RollbackMainchainTx(batch, mainchainTxHash)
	if err != nil {
		t.Error("Rollback the mainchain Tx failed")
	}

	batch.Commit()
	testChainStore.Close()
}
