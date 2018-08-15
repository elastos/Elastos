package blockchain

import (
	"container/list"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/core"

	"bytes"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

var testChainStore *ChainStore
var mainchainTxHash common.Uint256

func newTestChainStore() (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	st, err := NewLevelDB("Chain_UnitTest")
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		IStore:             st,
		headerIndex:        map[uint32]common.Uint256{},
		headerCache:        map[common.Uint256]*core.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	go store.loop()
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
	testChainStore.PersistMainchainTx(mainchainTxHash)

	// Need batch commit here because PersistMainchainTx use BatchPut
	testChainStore.BatchCommit()

	// 3. Verify PersistMainchainTx
	exist, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Not found the mainchain Tx")
	}
	if exist != ValueExist {
		t.Error("Mainchian Tx matched wrong value")
	}
}

func TestChainStore_PersistRegisterIdentificationTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	txHash1 := common.Uint256{1, 1, 1}
	txHash2 := common.Uint256{2, 2, 2}

	buf := new(bytes.Buffer)
	buf.WriteString("id")
	buf.WriteString("path")

	// 1. The register identification Tx should not exist in DB.
	_, err := testChainStore.GetRegisterIdentificationTx(buf.Bytes())
	if err == nil {
		t.Error("Found the register identification Tx which should not exist in DB")
	}

	// 2. Run PersistRegisterIdentificationTx
	testChainStore.PersistRegisterIdentificationTx(buf.Bytes(), txHash1)

	// Need batch commit here because PersistRegisterIdentificationTx use BatchPut
	testChainStore.BatchCommit()

	// 3. Verify PersistRegisterIdentificationTx
	txHashBytes, err := testChainStore.GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		t.Error("Not found the register Tx")
	}
	txHash3, err := common.Uint256FromBytes(txHashBytes)
	if err != nil {
		t.Error("Invalid tx hash")
	}
	if !txHash3.IsEqual(txHash1) {
		t.Error("Register Tx matched wrong value")
	}

	// 4. Run PersistRegisterIdentificationTx again
	testChainStore.PersistRegisterIdentificationTx(buf.Bytes(), txHash2)

	// Need batch commit here because PersistRegisterIdentificationTx use BatchPut
	testChainStore.BatchCommit()

	// 5. Verify PersistRegisterIdentificationTx again
	txHashBytes, err = testChainStore.GetRegisterIdentificationTx(buf.Bytes())
	if err != nil {
		t.Error("Not found the register Tx")
	}
	txHash4, err := common.Uint256FromBytes(txHashBytes)
	if err != nil {
		t.Error("Invalid tx hash")
	}
	if !txHash4.IsEqual(txHash2) {
		t.Error("Register Tx matched wrong value")
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
	err = testChainStore.RollbackMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Rollback the mainchain Tx failed")
	}

	// Need batch commit here because RollbackMainchainTx use BatchDelete
	testChainStore.BatchCommit()

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
	testChainStore.PersistMainchainTx(mainchainTxHash)

	// Need batch commit here because PersistMainchainTx use BatchPut
	testChainStore.BatchCommit()

	// 3. Verify PersistMainchainTx
	exist, err := testChainStore.GetMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Not found the mainchain Tx")
	}
	if exist != ValueExist {
		t.Error("Mainchian Tx matched wrong value")
	}

	// 4. Run IsMainchainTxHashDuplicate
	isDuplicate := testChainStore.IsMainchainTxHashDuplicate(mainchainTxHash)
	if !isDuplicate {
		t.Error("Mainchain Tx hash should be checked to be duplicated")
	}
}

func TestChainStoreDone(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
		return
	}

	err := testChainStore.RollbackMainchainTx(mainchainTxHash)
	if err != nil {
		t.Error("Rollback the mainchain Tx failed")
	}

	testChainStore.BatchCommit()
	testChainStore.Close()
}
