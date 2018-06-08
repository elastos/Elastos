package blockchain

import (
	"container/list"
	"testing"

	ela "github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var store *ChainStore
var sidechainTxHash string

func newTestChainStore() (*ChainStore, error) {
	// TODO: read config file decide which db to use.
	st, err := NewLevelDB("Chain_UnitTest")
	if err != nil {
		return nil, err
	}

	store := &ChainStore{
		IStore:             st,
		headerIndex:        map[uint32]common.Uint256{},
		headerCache:        map[common.Uint256]*ela.Header{},
		headerIdx:          list.New(),
		currentBlockHeight: 0,
		storedHeaderCount:  0,
		taskCh:             make(chan persistTask, TaskChanCap),
		quit:               make(chan chan bool, 1),
	}

	store.NewBatch()

	return store, nil
}

func TestInit(t *testing.T) {
	// Get new chainstore
	var err error
	store, err = newTestChainStore()
	if err != nil {
		t.Error("Create chainstore failed")
	}

	// Assume the sidechain Tx hash
	sidechainTxHash = "39fc8ba05b0064381e51afed65b4cf91bb8db60efebc38242e965d1b1fed0701"
}

func TestChainStore_PersisSidechainTx(t *testing.T) {
	if store == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx should not exist in DB.
	_, err := store.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should not exist in DB")
	}

	// 2. Run PersistSidechainTx
	store.PersistSidechainTx(sidechainTxHash)

	// Need batch commit here because PersistSidechainTx use BatchPut
	store.BatchCommit()

	// 3. Verify PersistSidechainTx
	exist, err := store.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}
}

func TestChainStore_RollbackSidechainTx(t *testing.T) {
	if store == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx hash should exist in DB.
	exist, err := store.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}

	// 2. Run Rollback
	err = store.RollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	// Need batch commit here because RollbackSidechainTx use BatchDelete
	store.BatchCommit()

	// 3. Verify RollbackSidechainTx
	_, err = store.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should been deleted")
	}
}

func TestChainStore_IsSidechainTxHashDuplicate(t *testing.T) {
	if store == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx should not exist in DB.
	_, err := store.GetSidechainTx(sidechainTxHash)
	if err == nil {
		t.Error("Found the sidechain Tx which should not exist in DB")
	}

	// 2. Persist the sidechain Tx hash
	store.PersistSidechainTx(sidechainTxHash)

	// Need batch commit here because PersistSidechainTx use BatchPut
	store.BatchCommit()

	// 3. Verify PersistSidechainTx
	exist, err := store.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}

	// 4. Run IsSidechainTxHashDuplicate
	isDuplicate := store.IsSidechainTxHashDuplicate(sidechainTxHash)
	if !isDuplicate {
		t.Error("Sidechain Tx hash should be checked to be duplicated")
	}
}

func TestDone(t *testing.T) {
	if store == nil {
		t.Error("Chainstore init failed")
	}

	err := store.RollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	store.BatchCommit()
}
