package blockchain

import (
	"container/list"
	"testing"

	ela "github.com/elastos/Elastos.ELA.SideChain/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var testChainStore *ChainStore
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

	// Assume the sidechain Tx hash
	sidechainTxHash = "39fc8ba05b0064381e51afed65b4cf91bb8db60efebc38242e965d1b1fed0701"
}

func TestChainStoreDone(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
		return
	}

	testChainStore.Close()
}
