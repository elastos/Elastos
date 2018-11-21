package blockchain

import (
	"container/list"
	"testing"

	ela "github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

var testChainStore *ChainStore
var sidechainTxHash common.Uint256

func NewTestChainStore() (*ChainStore, error) {
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
		producerVotes:      make(map[common.Uint168]*ProducerInfo, 0),
	}

	go store.loop()
	store.NewBatch()

	return store, nil
}

func TestChainStoreInit(t *testing.T) {
	// Get new chainstore
	var err error
	testChainStore, err = NewTestChainStore()
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
	exist, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}
}

func TestChainStore_RollbackSidechainTx(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1. The sidechain Tx hash should exist in DB.
	exist, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}

	// 2. Run Rollback
	err = testChainStore.RollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	// Need batch commit here because RollbackSidechainTx use BatchDelete
	testChainStore.BatchCommit()

	// 3. Verify RollbackSidechainTx
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
	exist, err := testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
	}
	if exist != ValueExist {
		t.Error("Sidechian Tx matched wrong value")
	}

	// 4. Run IsSidechainTxHashDuplicate
	isDuplicate := testChainStore.IsSidechainTxHashDuplicate(sidechainTxHash)
	if !isDuplicate {
		t.Error("Sidechain Tx hash should be checked to be duplicated")
	}
}

func TestChainStore_PersistRegisterProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	// addr: EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda
	publicKey1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	nickName1 := "nickname 1"
	payload1 := &ela.PayloadRegisterProducer{
		PublicKey: publicKey1,
		NickName:  nickName1,
		Url:       "http://www.test.com",
		Location:  1,
	}

	// addr: EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD
	publicKey2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	nickName2 := "nickname 2"
	payload2 := &ela.PayloadRegisterProducer{
		PublicKey: publicKey2,
		NickName:  nickName2,
		Url:       "http://www.test.com",
		Location:  2,
	}

	// 2. Should have no producer in db
	producers := testChainStore.GetRegisteredProducers()
	if len(producers) != 0 {
		t.Error("Found registered producers in DB")
	}

	// 3. Run RegisterProducer
	err := testChainStore.PersistRegisterProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 4. Run RegisterProducer
	err = testChainStore.PersistRegisterProducer(payload2)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	producers = testChainStore.GetRegisteredProducers()
	if len(producers) != 2 {
		t.Error("GetRegisteredProducers failed")
	}

	// 5. check
	if producers[0].NickName != nickName1 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[0].PublicKey != publicKey1 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[1].NickName != nickName2 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[1].PublicKey != publicKey2 {
		t.Error("GetRegisteredProducers failed")
	}
}

func TestChainStore_PersistCancelProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	// addr: EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda
	publicKey1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	payload1 := &ela.PayloadCancelProducer{
		PublicKey: publicKey1,
	}

	// addr: EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD
	publicKey2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	nickName2 := "nickname 2"

	// 2. Run RegisterProducer
	err := testChainStore.PersistCancelProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 3. Run GetRegisteredProducers
	producers := testChainStore.GetRegisteredProducers()
	if len(producers) != 1 {
		t.Error("GetRegisteredProducers failed")
	}

	// 4. Check payload
	if producers[0].NickName != nickName2 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[0].PublicKey != publicKey2 {
		t.Error("GetRegisteredProducers failed")
	}
}

func TestChainStore_PersistVoteProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data

	addr2 := "EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD"
	publicKey2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	nickName2 := "nickname 2"
	stake1 := common.Fixed64(110000000)
	payload1 := &ela.PayloadVoteProducer{
		Voter:      "voter",
		Stake:      stake1,
		PublicKeys: []string{publicKey2},
	}

	// 2. Run PersistVoteProducer
	err := testChainStore.PersistVoteProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 3. Run GetRegisteredProducers
	producers := testChainStore.GetRegisteredProducers()
	if len(producers) != 1 {
		t.Error("GetRegisteredProducers failed")
	}

	// 4. Check payload
	if producers[0].NickName != nickName2 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[0].PublicKey != publicKey2 {
		t.Error("GetRegisteredProducers failed")
	}

	// 5. Run GetProducerVote
	programHash, _ := common.Uint168FromAddress(addr2)
	vote1 := testChainStore.GetProducerVote(*programHash)
	if vote1 != stake1 {
		t.Error("GetProducerVote failed")
	}

	// 6. Run PersistVoteProducer
	err = testChainStore.PersistVoteProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 7.
	vote2 := testChainStore.GetProducerVote(*programHash)
	if vote2 != stake1*2 {
		t.Error("GetProducerVote failed")
	}

}

func TestChainStoreDone(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	err := testChainStore.RollbackSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Rollback the sidechain Tx failed")
	}

	testChainStore.BatchCommit()
}
