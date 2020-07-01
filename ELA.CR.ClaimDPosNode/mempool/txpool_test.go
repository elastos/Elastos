// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package mempool

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	dplog "github.com/elastos/Elastos.ELA/dpos/log"
	"github.com/elastos/Elastos.ELA/dpos/state"
	elaerr "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

var (
	txPool        *TxPool
	initialLedger *blockchain.Ledger
	utxoCacheDB   = NewUtxoCacheDB()
)

type UtxoCacheDB struct {
	transactions map[common.Uint256]*types.Transaction
}

func (s *UtxoCacheDB) GetTransaction(txID common.Uint256) (
	*types.Transaction, uint32, error) {
	txn, exist := s.transactions[txID]
	if exist {
		return txn, 0, nil
	}
	return nil, 0, errors.New("leveldb: not found")
}

func (s *UtxoCacheDB) PutTransaction(txn *types.Transaction) {
	s.transactions[txn.Hash()] = txn
}

func (s *UtxoCacheDB) RemoveTransaction(txID common.Uint256) {
	delete(s.transactions, txID)
}

func NewUtxoCacheDB() *UtxoCacheDB {
	var db UtxoCacheDB
	db.transactions = make(map[common.Uint256]*types.Transaction)
	return &db
}

func TestTxPoolInit(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)
	dplog.Init("elastos", 0, 0, 0)

	params := &config.DefaultParams
	blockchain.FoundationAddress = params.Foundation
	chainStore, err := blockchain.NewChainStore(test.DataPath, params)
	if err != nil {
		t.Fatal("open LedgerStore err:", err)
		os.Exit(1)
	}

	arbitratorsPublicKeys := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	arbiters := make([]state.ArbiterMember, 0)
	for _, arbiter := range arbitratorsPublicKeys {
		arbiterByte, _ := common.HexStringToBytes(arbiter)
		ar, _ := state.NewOriginArbiter(state.Origin, arbiterByte)
		arbiters = append(arbiters, ar)
	}
	arbitrators := state.NewArbitratorsMock(arbiters, 0, 3)

	chain, err := blockchain.New(chainStore, params, state.NewState(params, nil,
		nil, nil, nil), nil)
	if err != nil {
		t.Fatal(err, "BlockChain generate failed")
	}
	chain.UTXOCache = blockchain.NewUTXOCache(utxoCacheDB, &config.DefaultParams)
	err = chain.Init(nil)
	assert.NoError(t, err)
	initialLedger = blockchain.DefaultLedger
	blockchain.DefaultLedger = &blockchain.Ledger{
		Blockchain:  chain,
		Store:       chainStore,
		Arbitrators: arbitrators,
	}

	txPool = NewTxPool(&config.DefaultParams)
}

func TestTxPool_VerifyDuplicateSidechainTx(t *testing.T) {
	hashStr1 := "8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62"
	hashBytes1, _ := common.HexStringToBytes(hashStr1)
	hash1, _ := common.Uint256FromBytes(hashBytes1)
	hashStr2 := "cc62e14f5f9526b7f4ff9d34dcd0643dacb7886707c57f49ec97b95ec5c4edac"
	hashBytes2, _ := common.HexStringToBytes(hashStr2)
	hash2, _ := common.Uint256FromBytes(hashBytes2)

	// 1. Generate a withdraw transaction
	txn1 := new(types.Transaction)
	txn1.TxType = types.WithdrawFromSideChain
	txn1.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash1,
			*hash2,
		},
	}

	// 2. Add sidechain Tx to pool
	assert.NoError(t, txPool.AppendTx(txn1))

	// 3. Generate a withdraw transaction with duplicate sidechain Tx which already in the pool
	txn2 := new(types.Transaction)
	txn2.TxType = types.WithdrawFromSideChain
	txn2.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash1, // duplicate sidechain Tx
		},
	}

	// 4. Run verifyDuplicateSidechainTx
	assert.Errorf(t, txPool.VerifyTx(txn2),
		"Should find the duplicate sidechain tx")
}

func TestTxPool_VerifyDuplicateCRTx(t *testing.T) {
	// 1. Generate a register CR transaction
	tx1 := new(types.Transaction)
	tx1.TxType = types.TransferAsset
	tx1.Payload = &payload.TransferAsset{}
	tx1.Outputs = []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{1, 2, 3},
			Value:       1,
			ProgramHash: common.Uint168{1, 2, 3},
		},
		&types.Output{
			AssetID:     common.Uint256{4, 5, 6},
			Value:       1,
			ProgramHash: common.Uint168{4, 5, 6},
		},
	}
	tx2 := new(types.Transaction)
	tx2.TxType = types.TransferAsset
	tx2.Payload = &payload.TransferAsset{}
	tx2.Outputs = []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{11, 12, 13},
			Value:       1,
			ProgramHash: common.Uint168{11, 12, 13},
		},
		&types.Output{
			AssetID:     common.Uint256{14, 15, 16},
			Value:       1,
			ProgramHash: common.Uint168{14, 15, 16},
		},
	}

	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	pk1, _ := crypto.DecodePoint(publicKey1)
	ct1, _ := contract.CreateStandardContract(pk1)
	hash1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)
	input1 := &types.Input{
		Previous: types.OutPoint{
			TxID:  tx1.Hash(),
			Index: 0,
		},
		Sequence: 0,
	}
	input2 := &types.Input{
		Previous: types.OutPoint{
			TxID:  tx2.Hash(),
			Index: 0,
		},
		Sequence: 0,
	}
	input3 := &types.Input{
		Previous: types.OutPoint{
			TxID:  tx1.Hash(),
			Index: 1,
		},
		Sequence: 0,
	}
	input4 := &types.Input{
		Previous: types.OutPoint{
			TxID:  tx2.Hash(),
			Index: 1,
		},
		Sequence: 0,
	}

	tx3 := new(types.Transaction)
	tx3.TxType = types.RegisterCR
	tx3.Version = types.TxVersion09
	tx3.Payload = &payload.CRInfo{
		Code:     ct1.Code,
		CID:      *hash1,
		NickName: "nickname 1",
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	tx3.Inputs = []*types.Input{input1}

	tx4 := new(types.Transaction)
	tx4.TxType = types.UpdateCR
	tx4.Version = types.TxVersion09
	tx4.Payload = &payload.CRInfo{
		Code:     ct1.Code,
		CID:      *hash1,
		NickName: "nickname 2",
		Url:      "http://www.elastos_test.com",
		Location: 2,
	}
	tx4.Inputs = []*types.Input{input2}

	tx5 := new(types.Transaction)
	tx5.TxType = types.RegisterProducer
	tx5.Version = types.TxVersion09
	tx5.Payload = &payload.ProducerInfo{
		OwnerPublicKey: publicKey1,
		NodePublicKey:  publicKey2,
		NickName:       "nickname 3",
		Url:            "http://www.elastos_test.com",
		Location:       3,
	}
	tx5.Inputs = []*types.Input{input3}

	tx6 := new(types.Transaction)
	tx6.TxType = types.RegisterProducer
	tx6.Version = types.TxVersion09
	tx6.Payload = &payload.ProducerInfo{
		OwnerPublicKey: publicKey2,
		NodePublicKey:  publicKey1,
		NickName:       "nickname 4",
		Url:            "http://www.elastos_test.com",
		Location:       4,
	}
	tx6.Inputs = []*types.Input{input4}

	//tx7 tx8 no use same code,so not conflict
	tx7 := new(types.Transaction)
	tx7.TxType = types.ReturnDepositCoin
	tx7.Version = types.TxVersion09
	tx7.Programs = []*program.Program{
		&program.Program{
			Code:      []byte{11},
			Parameter: nil,
		},
	}

	tx8 := new(types.Transaction)
	tx8.TxType = types.ReturnCRDepositCoin
	tx8.Version = types.TxVersion09
	tx8.Programs = []*program.Program{
		&program.Program{
			Code:      []byte{22},
			Parameter: nil,
		},
	}

	// 2. Add tx1 and tx2 into store and input UTXO list
	utxoCacheDB.PutTransaction(tx1)
	utxoCacheDB.PutTransaction(tx2)

	// 3. Verify CR related tx
	assert.NoError(t, txPool.VerifyTx(tx3))
	assert.NoError(t, txPool.AppendTx(tx3))

	// 4. Verify duplicate CR related tx
	assert.Error(t, txPool.VerifyTx(tx4))

	// 5. Verify duplicate producer related tx
	assert.Error(t, txPool.VerifyTx(tx5))

	// 6. Verify duplicate producer related tx
	assert.Error(t, txPool.VerifyTx(tx6))

	// 7. Clean CR related tx
	txs := make([]*types.Transaction, 1)
	txs[0] = tx3
	txPool.cleanTransactions(txs)

	// 8. Verify duplicate producer related tx
	assert.NoError(t, txPool.VerifyTx(tx5))
	assert.NoError(t, txPool.AppendTx(tx5))

	// 9. Verify CR related tx
	assert.Error(t, txPool.VerifyTx(tx3))

	// 10. Verify CR related tx
	assert.NoError(t, txPool.VerifyTx(tx4))

	// 11. Clean producer related tx
	txs2 := make([]*types.Transaction, 2)
	txs2[0] = tx4
	txs2[1] = tx5
	txPool.cleanTransactions(txs2)

	// 12. Verify CR related tx
	assert.NoError(t, txPool.VerifyTx(tx3))

	// 13. Verify ReturnDepositCoin tx
	assert.NoError(t, txPool.VerifyTx(tx7))
	assert.NoError(t, txPool.AppendTx(tx7))

	// 14. Verify same ReturnDepositCoin tx again
	assert.Error(t, txPool.VerifyTx(tx7))

	// 15. Verify ReturnCRDepositCoin tx
	assert.NoError(t, txPool.VerifyTx(tx8))
	assert.NoError(t, txPool.AppendTx(tx8))

	// 16. Verify same ReturnCRDepositCoin tx again
	assert.Error(t, txPool.VerifyTx(tx8))

	txs3 := make([]*types.Transaction, 2)
	txs3[0] = tx7
	txs3[1] = tx8
	txPool.cleanTransactions(txs3)

	//tx9 tx10 both use ct1.code should conflict
	tx9 := new(types.Transaction)
	tx9.TxType = types.ReturnDepositCoin
	tx9.Version = types.TxVersion09
	tx9.Programs = []*program.Program{
		&program.Program{
			Code:      ct1.Code,
			Parameter: nil,
		},
	}

	tx10 := new(types.Transaction)
	tx10.TxType = types.ReturnCRDepositCoin
	tx10.Version = types.TxVersion09
	tx10.Programs = []*program.Program{
		&program.Program{
			Code:      ct1.Code,
			Parameter: nil,
		},
	}
	// 13. Verify ReturnDepositCoin tx
	assert.NoError(t, txPool.VerifyTx(tx9))
	assert.NoError(t, txPool.AppendTx(tx9))

	// 14. Verify same ReturnDepositCoin tx again
	assert.Error(t, txPool.VerifyTx(tx9))

	// 16. Verify same ReturnCRDepositCoin tx again
	assert.Error(t, txPool.VerifyTx(tx10))

}

func TestTxPool_CleanSidechainTx(t *testing.T) {
	hashStr1 := "300db7783393a6f60533c1223108445df57de4fb4842f84f55d07df57caa0c7d"
	hashBytes1, _ := common.HexStringToBytes(hashStr1)
	hash1, _ := common.Uint256FromBytes(hashBytes1)
	hashStr2 := "d6c2cb8345a8fe4af0d103cc4e40dbb0654bb169a85bb8cc57923d0c72f3658f"
	hashBytes2, _ := common.HexStringToBytes(hashStr2)
	hash2, _ := common.Uint256FromBytes(hashBytes2)
	hashStr3 := "326218253e6feaa21e3521eff27418b942a5fbd45347505f3e5aca0463baffe2"
	hashBytes3, _ := common.HexStringToBytes(hashStr3)
	hash3, _ := common.Uint256FromBytes(hashBytes3)
	hashStr4 := "645b614eaaa0a1bfd7015d88f3c1343048343924fc105e403b735ba754caa8db"
	hashBytes4, _ := common.HexStringToBytes(hashStr4)
	hash4, _ := common.Uint256FromBytes(hashBytes4)
	hashStr5 := "9dcad6d4ec2851bf522ddd301c7567caf98554a82a0bcce866de80b503909642"
	hashBytes5, _ := common.HexStringToBytes(hashStr5)
	hash5, _ := common.Uint256FromBytes(hashBytes5)

	// 1. Generate some withdraw transactions
	txn1 := new(types.Transaction)
	txn1.TxType = types.WithdrawFromSideChain
	txn1.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash1,
			*hash2,
		},
	}

	txn2 := new(types.Transaction)
	txn2.TxType = types.WithdrawFromSideChain
	txn2.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash3,
		},
	}

	txn3 := new(types.Transaction)
	txn3.TxType = types.WithdrawFromSideChain
	txn3.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash4,
			*hash5,
		},
	}
	txns := []*types.Transaction{txn1, txn2, txn3}

	// 2. Add to sidechain txs pool
	for _, txn := range txns {
		assert.NoError(t, txPool.AppendTx(txn))
	}

	// Verify sidechain tx pool state
	for _, txn := range txns {
		assert.Error(t, txPool.VerifyTx(txn),
			"Should find the duplicate sidechain tx")
	}

	// 3. Run cleanSidechainTx
	for _, txn := range txns {
		assert.NoError(t, txPool.removeTx(txn))
	}

	// Verify sidechian tx pool state
	for _, txn := range txns {
		assert.NoError(t, txPool.VerifyTx(txn),
			"Should not find the duplicate sidechain tx")
	}
}

func TestTxPool_ReplaceDuplicateSideChainPowTx(t *testing.T) {
	var sideBlockHash1 common.Uint256
	var sideBlockHash2 common.Uint256
	var sideGenesisHash common.Uint256
	rand.Read(sideBlockHash1[:])
	rand.Read(sideBlockHash2[:])
	rand.Read(sideGenesisHash[:])

	txn1 := new(types.Transaction)
	txn1.TxType = types.SideChainPow
	txn1.Payload = &payload.SideChainPow{
		SideBlockHash:   sideBlockHash1,
		SideGenesisHash: sideGenesisHash,
		BlockHeight:     100,
	}

	txPool.txnList[txn1.Hash()] = txn1

	txn2 := new(types.Transaction)
	txn2.TxType = types.SideChainPow
	txn2.Payload = &payload.SideChainPow{
		SideBlockHash:   sideBlockHash2,
		SideGenesisHash: sideGenesisHash,
		BlockHeight:     100,
	}
	txPool.replaceDuplicateSideChainPowTx(txn2)
	txPool.txnList[txn2.Hash()] = txn2

	if txn := txPool.GetTransaction(txn1.Hash()); txn != nil {
		t.Errorf("Txn1 should be replaced")
	}

	if txn := txPool.GetTransaction(txn2.Hash()); txn == nil {
		t.Errorf("Txn2 should be added in txpool")
	}
}

func TestTxPool_IsDuplicateSidechainTx(t *testing.T) {
	var sideTx1 common.Uint256
	var sideTx2 common.Uint256
	rand.Read(sideTx1[:])
	rand.Read(sideTx2[:])

	// 1. Generate a withdraw transaction
	txn1 := new(types.Transaction)
	txn1.TxType = types.WithdrawFromSideChain
	txn1.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			sideTx1,
			sideTx2,
		},
	}

	// 2. Add sidechain Tx to pool
	assert.NoError(t, txPool.AppendTx(txn1))

	// 3. Run IsDuplicateSidechainTx
	inPool := txPool.IsDuplicateSidechainTx(sideTx1)
	if !inPool {
		t.Error("Should find duplicate sidechain tx")
	}
}

func TestTxPool_AppendToTxnPool(t *testing.T) {
	tx := new(types.Transaction)
	txBytes, _ := hex.DecodeString("000403454c41010008803e6306563b26de010" +
		"000000000000000000000000000000000000000000000000000000000000000ffff" +
		"ffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a" +
		"4ead0a39becdc01000000000000000012c8a2e0677227144df822b7d9246c58df68" +
		"eb11ceb037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead" +
		"0a3c1d258040000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e5" +
		"06dd00000000")
	tx.Deserialize(bytes.NewReader(txBytes))
	errCode := txPool.AppendToTxPool(tx)
	assert.Equal(t, errCode.Code(), elaerr.ErrBlockIneffectiveCoinbase)
}

func TestTxPool_CleanSubmittedTransactions(t *testing.T) {
	appendTx := func(tx *types.Transaction) elaerr.ELAError {
		if err := txPool.AppendTx(tx); err != nil {
			return err
		}
		txPool.txnList[tx.Hash()] = tx
		return nil
	}

	txPool = NewTxPool(&config.DefaultParams)
	/*------------------------------------------------------------*/
	/* check double spend but not duplicate txs */
	//two mock transactions, they are double-spent to each other.
	tx1Prev := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{1, 1, 1},
				Value:       1,
				ProgramHash: common.Uint168{1, 1, 1},
			},
		},
	}
	input := &types.Input{
		Previous: types.OutPoint{
			TxID:  tx1Prev.Hash(),
			Index: 0,
		},
		Sequence: 100,
	}
	utxoCacheDB.PutTransaction(tx1Prev)
	tx1 := new(types.Transaction)
	tx1.TxType = types.TransferAsset
	tx1.PayloadVersion = 0
	tx1.Payload = &payload.TransferAsset{}
	var attribute1 *types.Attribute
	attribute1 = &types.Attribute{
		Usage: types.Nonce,
		Data: []byte("5217023ca4139475f8a4c2772a113168568da958c05faaaedff1b3" +
			"77d420ed328f39f15420f48ce4e9c83b69b14e88da00ab6c87f35dc5841c064" +
			"35b7c49dbf3a944171e3d8604dd817324bb2c77f0500000ae0858a6c4222a83" +
			"ba0c42ea3d8038177531a4dfc8183a0ab1de6741e6da79b8bddeacdeeefb78f" +
			"586c8bc45e9c"),
	}
	tx1.Attributes = []*types.Attribute{attribute1}

	tx1.Inputs = []*types.Input{input}

	tx2 := new(types.Transaction)
	tx2.TxType = types.TransferAsset
	tx2.PayloadVersion = 0
	tx2.Payload = &payload.TransferAsset{}
	var attribute2 *types.Attribute
	attribute2 = &types.Attribute{
		Usage: types.Nonce,
		Data: []byte("202bf0908cfe9687d04f4dc29f3b73eea8d0f7b00d159a3f4843a4" +
			"400a86297404bda1c1f2f5c497149db3fdea371f1bb9e71c86dafccce128944" +
			"b26a7181ebafa9e4869cdfbc7a6e1f34b8818a78f361888907452a05d04c399" +
			"1c10e92b1041e7258611dc52059917f4a946ea89cf68b7af0808e89aa5d8241" +
			"e453410fb1f46"),
	}
	tx2.Attributes = []*types.Attribute{attribute2}
	tx2.Inputs = []*types.Input{input}

	// a mock block
	var newBLock types.Block
	var previousBlockHash common.Uint256
	var merkleRoot common.Uint256
	var blockAuxpow auxpow.AuxPow
	blockAuxpow.Deserialize(bytes.NewReader([]byte("01000000010000000000000000" +
		"000000000000000000000000000000000000000000000000000000002cfabe6d6d0" +
		"5282102a9ced24c5d8260407b8685f57ec3e9485e00a17d9a43d66f90e776aa0100" +
		"0000000000000000000000000000000000000000000000000000000000000000000" +
		"00000000000000000000000000000000000000000000000ffffff7f000000000000" +
		"000000000000000000000000000000000000000000000000000029a6f8a6f4b265a" +
		"4b96f83a570025c07552480934ca17ccbac69d43db7331bd86229275b0000000003" +
		"000000")))
	previousBlockHash.Deserialize(bytes.NewReader([]byte("5570625560dcd24ceeb8a5758aafd5a66045c159b5b00edcbaec59566b4d65bf")))
	merkleRoot.Deserialize(bytes.NewReader([]byte("0cd26e5ef833e469ed0e0df7cdc7b22f4cf294492c450e677c8a47846afecf22")))
	newBLock.Version = 0
	newBLock.Previous = previousBlockHash
	newBLock.MerkleRoot = merkleRoot
	newBLock.Timestamp = 1529293192
	newBLock.Bits = 545259519
	newBLock.Nonce = 0
	newBLock.Height = 221
	newBLock.AuxPow = blockAuxpow
	newBLock.Transactions = []*types.Transaction{tx2}
	assert.NoError(t, appendTx(tx1))

	txPool.CleanSubmittedTransactions(&newBLock)

	_, exist := txPool.txnList[tx1.Hash()]
	assert.False(t, exist,
		"Should delete double spent utxo transaction")

	for _, input := range tx1.Inputs {
		assert.True(t, txPool.getInputUTXOList(input) == nil,
			"Should delete double spent utxo transaction")
	}
	/*------------------------------------------------------------*/
	/* check duplicated sidechain hashes */
	// re-initialize the tx pool
	var sideBlockHash1 common.Uint256
	var sideBlockHash2 common.Uint256
	var sideBlockHash3 common.Uint256
	var sideBlockHash4 common.Uint256
	var sideBlockHash5 common.Uint256

	rand.Read(sideBlockHash1[:])
	fmt.Println("sideBlockHash1:", sideBlockHash1)
	rand.Read(sideBlockHash2[:])
	fmt.Println("sideBlockHash2:", sideBlockHash2)
	rand.Read(sideBlockHash3[:])
	fmt.Println("sideBlockHash3:", sideBlockHash3)
	rand.Read(sideBlockHash4[:])
	fmt.Println("sideBlockHash4:", sideBlockHash4)
	rand.Read(sideBlockHash5[:])
	fmt.Println("sideBlockHash5:", sideBlockHash5)

	txPool = NewTxPool(&config.DefaultParams)
	//two mock transactions again, they have some identical sidechain hashes
	tx3Prev := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{3, 3, 3},
				Value:       1,
				ProgramHash: common.Uint168{3, 3, 3},
			},
		},
	}
	utxoCacheDB.PutTransaction(tx3Prev)
	tx3 := new(types.Transaction)
	tx3.TxType = types.WithdrawFromSideChain
	tx3.Payload = &payload.WithdrawFromSideChain{
		SideChainTransactionHashes: []common.Uint256{sideBlockHash1, sideBlockHash2},
	}
	tx3.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  tx3Prev.Hash(),
				Index: 0,
			},
			Sequence: 100,
		},
	}
	tx4Prev := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{4, 4, 4},
				Value:       1,
				ProgramHash: common.Uint168{4, 4, 4},
			},
		},
	}
	utxoCacheDB.PutTransaction(tx4Prev)
	tx4 := new(types.Transaction)
	tx4.TxType = types.WithdrawFromSideChain
	tx4.Payload = &payload.WithdrawFromSideChain{
		SideChainTransactionHashes: []common.Uint256{sideBlockHash1, sideBlockHash4},
	}
	tx4.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  tx4Prev.Hash(),
				Index: 0,
			},
			Sequence: 100,
		},
	}
	tx5Prev := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{5, 5, 5},
				Value:       1,
				ProgramHash: common.Uint168{5, 5, 5},
			},
		},
	}
	utxoCacheDB.PutTransaction(tx5Prev)
	tx5 := new(types.Transaction)
	tx5.TxType = types.WithdrawFromSideChain
	tx5.Payload = &payload.WithdrawFromSideChain{
		SideChainTransactionHashes: []common.Uint256{sideBlockHash2, sideBlockHash5},
	}
	tx5.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  tx5Prev.Hash(),
				Index: 0,
			},
			Sequence: 100,
		},
	}
	tx6Prev := &types.Transaction{
		TxType:  types.TransferAsset,
		Payload: &payload.TransferAsset{},
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{6, 6, 6},
				Value:       1,
				ProgramHash: common.Uint168{6, 6, 6},
			},
		},
	}
	utxoCacheDB.PutTransaction(tx6Prev)
	tx6 := new(types.Transaction)
	tx6.TxType = types.WithdrawFromSideChain
	tx6.Payload = &payload.WithdrawFromSideChain{
		SideChainTransactionHashes: []common.Uint256{sideBlockHash3},
	}
	tx6.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  tx6Prev.Hash(),
				Index: 0,
			},
			Sequence: 100,
		},
	}

	assert.NoError(t, appendTx(tx4))
	assert.NoError(t, appendTx(tx5))
	assert.NoError(t, appendTx(tx6))

	newBLock.Transactions = []*types.Transaction{tx4, tx5, tx6}
	txPool.CleanSubmittedTransactions(&newBLock)
	if err := isTransactionCleaned(txPool, tx4); err != nil {
		t.Error("should clean transaction tx4:", err)
	}

	if err := isTransactionCleaned(txPool, tx5); err != nil {
		t.Error("should clean transaction: tx5:", err)
	}

	if err := isTransactionCleaned(txPool, tx6); err != nil {
		t.Error("should have transaction: tx6", err)
	}

	/*------------------------------------------------------------*/
	/* check double spend and duplicate txs */
	txPool = NewTxPool(&config.DefaultParams)

	assert.NoError(t, appendTx(tx4))

	newBLock.Transactions = []*types.Transaction{tx4}

	txPool.CleanSubmittedTransactions(&newBLock)

	if err := isTransactionCleaned(txPool, tx4); err != nil {
		t.Error("should clean transaction tx4:", err)
	}

	/*------------------------------------------------------------*/
	/* normal case */
	assert.NoError(t, appendTx(tx2))
	newBLock.Transactions = []*types.Transaction{tx3}
	txPool.CleanSubmittedTransactions(&newBLock)
	txPool.CheckAndCleanAllTransactions()
	if err := isTransactionCleaned(txPool, tx2); err != nil {
		t.Error("should clean transaction: tx2", err)
	}
}

func TestTxPool_End(t *testing.T) {
	blockchain.DefaultLedger.Store.Close()
	blockchain.DefaultLedger = initialLedger
	initialLedger = nil
}

func isTransactionExisted(pool *TxPool, tx *types.Transaction) error {
	if _, ok := pool.txnList[tx.Hash()]; !ok {
		return fmt.Errorf("does not have transaction in transaction pool")
	}
	for _, input := range tx.Inputs {
		if poolInput := pool.getInputUTXOList(input); poolInput == nil {
			return fmt.Errorf("does not have utxo inputs in input list pool" + input.String())
		}
	}
	if tx.TxType == types.WithdrawFromSideChain {
		payload := tx.Payload.(*payload.WithdrawFromSideChain)
		for _, hash := range payload.SideChainTransactionHashes {
			if pool.ContainsKey(hash, slotSidechainTxHashes) {
				return fmt.Errorf("does not have sidechain hash in sidechain list pool" + hash.String())
			}
		}
	}
	return nil
}

func isTransactionCleaned(pool *TxPool, tx *types.Transaction) error {
	if tx := pool.txnList[tx.Hash()]; tx != nil {
		return fmt.Errorf("has transaction in transaction pool" + tx.Hash().String())
	}
	for _, input := range tx.Inputs {
		if poolInput := pool.getInputUTXOList(input); poolInput != nil {
			return fmt.Errorf("has utxo inputs in input list pool" + input.String())
		}
	}
	if tx.TxType == types.WithdrawFromSideChain {
		payload := tx.Payload.(*payload.WithdrawFromSideChain)
		for _, hash := range payload.SideChainTransactionHashes {
			if pool.ContainsKey(hash, slotSidechainTxHashes) {
				return fmt.Errorf("has sidechain hash in sidechain list pool" + hash.String())
			}
		}
	}
	return nil
}
