package blockchain

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/assert"
)

var testChainStore *ChainStore
var sidechainTxHash common.Uint256

func TestChainStoreInit(t *testing.T) {
	// Get new chainstore
	temp, err := NewChainStore("Chain_UnitTest")
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
	_, err = testChainStore.GetSidechainTx(sidechainTxHash)
	if err != nil {
		t.Error("Not found the sidechain Tx")
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
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	nickName1 := "nickname 1"
	payload1 := &payload.PayloadRegisterProducer{
		PublicKey: publicKey1,
		NickName:  nickName1,
		Url:       "http://www.test.com",
		Location:  1,
		Address:   "127.0.0.1",
	}

	// addr: EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	nickName2 := "nickname 2"
	payload2 := &payload.PayloadRegisterProducer{
		PublicKey: publicKey2,
		NickName:  nickName2,
		Url:       "http://www.test.com",
		Location:  2,
		Address:   "127.0.0.1",
	}

	// 2. Should have no producer in db
	producers, err := testChainStore.GetRegisteredProducersSorted()
	if err == nil && len(producers) != 0 {
		t.Error("Found registered producers in DB")
	}

	// 3. Run RegisterProducer
	err = testChainStore.PersistRegisterProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed:", err.Error())
	}
	testChainStore.BatchCommit()

	// 4. Run RegisterProducer
	err = testChainStore.PersistRegisterProducer(payload2)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	producers, err = testChainStore.GetRegisteredProducersSorted()
	if len(producers) != 2 {
		t.Error("GetRegisteredProducers failed")
	}

	// 5. check
	if producers[0].NickName != nickName1 {
		t.Error("GetRegisteredProducers failed")
	}
	if !bytes.Equal(producers[0].PublicKey, publicKey1) {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[1].NickName != nickName2 {
		t.Error("GetRegisteredProducers failed")
	}
	if !bytes.Equal(producers[1].PublicKey, publicKey2) {
		t.Error("GetRegisteredProducers failed")
	}
}

func TestChainStore_TransactionChecks(t *testing.T) {
	originChainStore := DefaultLedger.Store
	DefaultLedger.Store = testChainStore

	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	//publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	//publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	publicKeyStr3 := "03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613"
	publicKey3, _ := common.HexStringToBytes(publicKeyStr3)

	publicKeyPlege1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)
	publicKeyPlege3, _ := contract.PublicKeyToDepositProgramHash(publicKey3)

	//CheckRegisterProducerTransaction

	txn := new(types.Transaction)
	txn.TxType = types.RegisterProducer
	txn.Payload = &payload.PayloadRegisterProducer{
		PublicKey: publicKey1,
		NickName:  "nick name 1",
		Url:       "http://www.google.com",
		Location:  1,
		Address:   "127.0.0.1",
	}

	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	txn.Outputs = []*types.Output{{
		AssetID:     common.Uint256{},
		Value:       5000,
		OutputLock:  0,
		ProgramHash: *publicKeyPlege1,
	}}

	err := CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Duplicated public key.")

	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = publicKey3
	txn.Payload.(*payload.PayloadRegisterProducer).NickName = "nickname 1"
	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr3),
		Parameter: nil,
	}}
	txn.Outputs[0].ProgramHash = *publicKeyPlege3

	err = CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Duplicated nick name.")

	txn.Payload.(*payload.PayloadRegisterProducer).NickName = "nickname 3"
	err = CheckRegisterProducerTransaction(txn)
	assert.NoError(t, err)

	//CheckUpdateProducerTransaction

	txn.TxType = types.UpdateProducer
	txn.Payload = &payload.PayloadUpdateProducer{
		PublicKey: publicKey3,
		NickName:  "nick name 1",
		Url:       "http://www.google.com",
		Location:  1,
		Address:   "127.0.0.1",
	}

	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr3),
		Parameter: nil,
	}}

	txn.Outputs = []*types.Output{{
		AssetID:     common.Uint256{},
		Value:       5000,
		OutputLock:  0,
		ProgramHash: *publicKeyPlege3,
	}}

	err = CheckUpdateProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid producer.")

	txn.Payload.(*payload.PayloadUpdateProducer).PublicKey = publicKey1
	txn.Payload.(*payload.PayloadUpdateProducer).NickName = "nickname 1"
	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}
	txn.Outputs[0].ProgramHash = *publicKeyPlege1

	err = CheckUpdateProducerTransaction(txn)
	assert.NoError(t, err)

	txn.Payload.(*payload.PayloadUpdateProducer).NickName = "nickname 2"
	err = CheckUpdateProducerTransaction(txn)
	assert.EqualError(t, err, "Duplicated nick name.")

	txn.Payload.(*payload.PayloadUpdateProducer).NickName = "nickname 3"
	err = CheckUpdateProducerTransaction(txn)
	assert.NoError(t, err)

	//CheckCancelProducerTransaction
	txn.TxType = types.CancelProducer
	cancelPayload := &payload.PayloadCancelProducer{
		PublicKey: publicKey3,
	}
	txn.Payload = cancelPayload

	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr3),
		Parameter: nil,
	}}

	err = CheckCancelProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid producer.")

	cancelPayload.PublicKey = publicKey1
	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	err = CheckCancelProducerTransaction(txn)
	assert.NoError(t, err)

	DefaultLedger.Store = originChainStore
}

func TestChainStore_PersistCancelProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	// addr: EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	payload1 := &payload.PayloadCancelProducer{
		PublicKey: publicKey1,
	}

	// addr: EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	nickName2 := "nickname 2"

	// 2. Run PersistCancelProducer
	err := testChainStore.PersistCancelProducer(payload1)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 3. Run GetRegisteredProducers
	producers, err := testChainStore.GetRegisteredProducersSorted()
	if err != nil || len(producers) != 1 {
		t.Error("GetRegisteredProducers failed")
	}

	// 4. Check payload
	if producers[0].NickName != nickName2 {
		t.Error("GetRegisteredProducers failed")
	}
	if !bytes.Equal(producers[0].PublicKey, publicKey2) {
		t.Error("GetRegisteredProducers failed")
	}

	testChainStore.RemoveCanceledProducer(publicKey1)
	testChainStore.BatchCommit()
}

func TestChainStore_PersistUpdateProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	// addr: EUa2s2Wmc1quGDACEGKmm5qrFEAgoQK9AD
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	nickName1 := "nickname 1"
	ip1 := "168.192.1.1"
	payload1 := &payload.PayloadUpdateProducer{
		PublicKey: publicKey2,
		NickName:  nickName1,
		Url:       "http://www.test.com",
		Location:  2,
		Address:   ip1,
	}

	// 2. Run RegisterProducer
	err := testChainStore.PersistUpdateProducer(payload1)
	if err != nil {
		t.Error("PersistUpdateProducer failed")
	}
	testChainStore.BatchCommit()

	// 3. Run GetRegisteredProducers
	producers, err := testChainStore.GetRegisteredProducersSorted()
	if err != nil || len(producers) != 1 {
		t.Error("GetRegisteredProducers failed")
	}

	// 4. Check payload
	if !bytes.Equal(producers[0].PublicKey, publicKey2) {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[0].NickName != nickName1 {
		t.Error("GetRegisteredProducers failed")
	}
	if producers[0].Address != ip1 {
		t.Error("GetRegisteredProducers failed")
	}

	//todo add test about checking chain store within CheckUpdateProducerTransaction
}

func TestChainStore_PersistVoteProducer(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	publicKeyStr1 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	addr1 := "DftptovCBev9dLt2BRf8nqJGzDeoLFsCfF"
	programHash1, _ := common.Uint168FromAddress(addr1)
	stake1 := common.Fixed64(110000000)
	output := &types.Output{
		AssetID:     common.Uint256{},
		Value:       stake1,
		OutputLock:  0,
		ProgramHash: *programHash1,
		OutputType:  types.VoteOutput,
		OutputPayload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				outputpayload.VoteContent{
					VoteType: outputpayload.Delegate,
					Candidates: [][]byte{
						publicKey1,
					},
				},
			},
		},
	}
	publicKeyStr2 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	output2 := &types.Output{
		AssetID:     common.Uint256{},
		Value:       stake1,
		OutputLock:  0,
		ProgramHash: *programHash1,
		OutputType:  types.VoteOutput,
		OutputPayload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				outputpayload.VoteContent{
					VoteType: outputpayload.Delegate,
					Candidates: [][]byte{
						publicKey1,
						publicKey2,
					},
				},
			},
		},
	}

	// addr: EZwPHEMQLNBpP2VStF3gRk8EVoMM2i3hda
	nickName2 := "nickname 2"
	payload2 := &payload.PayloadRegisterProducer{
		PublicKey: publicKey2,
		NickName:  nickName2,
		Url:       "http://www.test.com",
		Location:  1,
		Address:   "127.0.0.1",
	}

	// 2. Run RegisterProducer
	err := testChainStore.PersistRegisterProducer(payload2)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}
	testChainStore.BatchCommit()

	// 3. Run PersistVoteProducer
	err = testChainStore.PersistVoteOutput(output)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}

	// 4. Check vote
	vote1 := testChainStore.GetProducerVote(publicKey1)
	if vote1 != stake1 {
		t.Error("GetProducerVote failed")
	}

	// 5. Run PersistVoteProducer
	err = testChainStore.PersistVoteOutput(output)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}

	// 6. Check vote
	vote2 := testChainStore.GetProducerVote(publicKey1)
	if vote2 != stake1*2 {
		t.Error("GetProducerVote failed")
	}

	// 7. Run PersistVoteProducer
	err = testChainStore.PersistVoteOutput(output2)
	if err != nil {
		t.Error("PersistRegisterProducer failed")
	}

	// 8. Check vote
	vote3 := testChainStore.GetProducerVote(publicKey1)
	if vote3 != stake1*3 {
		t.Error("GetProducerVote failed")
	}
	vote4 := testChainStore.GetProducerVote(publicKey2)
	if vote4 != stake1 {
		t.Error("GetProducerVote failed")
	}
}

func TestChainStore_PersistCancelVoteOutput(t *testing.T) {
	if testChainStore == nil {
		t.Error("Chainstore init failed")
	}

	// 1.Prepare data
	addr1 := "DftptovCBev9dLt2BRf8nqJGzDeoLFsCfF"
	publicKeyStr1 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	programHash1, _ := common.Uint168FromAddress(addr1)
	stake1 := common.Fixed64(110000000)
	output := &types.Output{
		AssetID:     common.Uint256{},
		Value:       stake1,
		OutputLock:  0,
		ProgramHash: *programHash1,
		OutputType:  types.VoteOutput,
		OutputPayload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				outputpayload.VoteContent{
					VoteType: outputpayload.Delegate,
					Candidates: [][]byte{
						publicKey1,
					},
				},
			},
		},
	}
	publicKeyStr2 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	output2 := &types.Output{
		AssetID:     common.Uint256{},
		Value:       stake1,
		OutputLock:  0,
		ProgramHash: *programHash1,
		OutputType:  types.VoteOutput,
		OutputPayload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				outputpayload.VoteContent{
					VoteType: outputpayload.Delegate,
					Candidates: [][]byte{
						publicKey1,
						publicKey2,
					},
				},
			},
		},
	}

	// 2. Run PersistCancelVoteOutput
	err := testChainStore.PersistCancelVoteOutput(output)
	if err != nil {
		t.Error("PersistCancelVoteOutput failed")
	}

	// 3. Check vote
	vote1 := testChainStore.GetProducerVote(publicKey1)
	if vote1 != stake1*2 {
		t.Error("GetProducerVote failed")
	}

	// 4. Run PersistCancelVoteOutput
	err = testChainStore.PersistCancelVoteOutput(output2)
	if err != nil {
		t.Error("PersistCancelVoteOutput failed")
	}

	// 5. Check Vote
	vote2 := testChainStore.GetProducerVote(publicKey2)
	if vote2 != 0 {
		t.Error("GetProducerVote failed")
	}

	testChainStore.ClearRegisteredProducer()
	testChainStore.BatchCommit()
}

func TestCheckAssetPrecision(t *testing.T) {
	originalStore := DefaultLedger.Store
	DefaultLedger.Store = testChainStore

	assetStr := "b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3"
	defaultAsset, _ := common.Uint256FromHexString(assetStr)

	// normal transaction
	tx := buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = *defaultAsset
		output.ProgramHash = common.Uint168{}
	}
	err := CheckAssetPrecision(tx)
	assert.NoError(t, err)

	// asset not exist
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = CheckAssetPrecision(tx)
	assert.EqualError(t, err, "The asset not exist in local blockchain.")

	// register asset
	asset := payload.Asset{
		Name:      "TEST",
		Precision: 0x04,
		AssetType: 0x00,
	}
	register := &types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &payload.PayloadRegisterAsset{
			Asset:  asset,
			Amount: 0 * 100000000,
		},
	}
	testChainStore.NewBatch()
	testChainStore.PersistAsset(register.Hash(), asset)
	testChainStore.BatchCommit()

	// valid precision
	for _, output := range tx.Outputs {
		output.AssetID = register.Hash()
		output.ProgramHash = common.Uint168{}
		output.Value = 123456780000
	}
	err = CheckAssetPrecision(tx)
	assert.NoError(t, err)

	// invalid precision
	for _, output := range tx.Outputs {
		output.AssetID = register.Hash()
		output.ProgramHash = common.Uint168{}
		output.Value = 12345678000
	}
	err = CheckAssetPrecision(tx)
	assert.EqualError(t, err, "The precision of asset is incorrect.")

	DefaultLedger.Store = originalStore
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
