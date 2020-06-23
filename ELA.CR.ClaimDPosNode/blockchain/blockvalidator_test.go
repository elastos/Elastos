// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"math"
	"math/rand"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/assert"
)

const (
	TestBlockHex = "000000007b3a8b2032301d0f9fafadee3bddba8d798a3ce1ed1574063ae3bb55628cec763a45dffe0f38d9efb5" +
		"0a41dbe6b7f4186ba9b4861ad624fdde6e1e775a81b0d3687f4c5add01561d000000001027000001000000010000000000000" +
		"000000000000000000000000000000000000000000000000000000000002cfabe6d6d6d126217acca4ed3b3aa40de6d1dad67" +
		"61a7bba4ebdb67c88714455cea580084010000000000000000000000000000000000000000000000000000000000000000000" +
		"0000000000000000000000000000000000000000000000000ffffff7f00000000000000000000000000000000000000000000" +
		"000000000000000000009fba1be4874f22da581831eb1a5243e53b51e57f3021222943a6a2919d19c19d687f4c5a000000001" +
		"28c95000102000000000403454c4101000847cfc35085f3aec001000000000000000000000000000000000000000000000000" +
		"0000000000000000ffffffffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3b54afb0" +
		"80000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e506b037db964a231458d2d6ffd5ea18944c4f90e63d54" +
		"7c5d3b9874df66a4ead0a3a803f5140000000000000000129e9cf1c5f336fcf3a6c954444ed482c5d916e5061027000000020" +
		"000016c3a8d6db4d3b4ccad1712a29c5e90e2e7bc26c603995fc18a37c85a5420ad445600ffffffff02b037db964a231458d2" +
		"d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3047823a7170100000000000021190ff3b12919c17f232db55431832" +
		"2a6b43ba372b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300b864d9450000000000000021" +
		"fa402bfaecabefacb6379c08edb5224fd95e25f700000000014140c72db63b7fdf90b8bf34e91f0a6394e25d1340f178a1776" +
		"bdc344fecf8ced8e4db627fb9ffa7068c51d3d15b92a749ffa407e2593833ec836d4cdaae1062abe52321035e1529938d1a36" +
		"bef97806557bdb4faec8c83a8fc557c1afb287b07bd923c589ac"
)

func init() {
	testing.Init()
}

func TestCheckBlockSanity(t *testing.T) {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)
	params := &config.DefaultParams
	FoundationAddress = params.Foundation
	chainStore, err := NewChainStore(filepath.Join(test.DataPath, "sanity"), params)
	if err != nil {
		t.Error(err.Error())
	}
	defer chainStore.Close()

	chain, _ := New(chainStore, params, state.NewState(params,
		nil, nil, nil, nil), nil)
	//chain.Init(nil)
	if DefaultLedger == nil {
		DefaultLedger = &Ledger{
			Blockchain: chain,
			Store:      chainStore,
		}
	}

	if err != nil {
		t.Error(err.Error())
	}

	timeSource := NewMedianTime()
	blockData, err := hex.DecodeString(TestBlockHex)
	if err != nil {
		t.Errorf("Decode block hex error %s", err.Error())
	}

	var block types.Block
	block.Deserialize(bytes.NewReader(blockData))
	err = chain.CheckBlockSanity(&block)
	if err != nil {
		t.Error(err.Error())
	}

	// change of time stamp, this will change the block hash
	// and the proof check would fail
	block.Timestamp = uint32(timeSource.AdjustedTime().Unix())
	err = chain.CheckBlockSanity(&block)
	assert.Error(t, err, "[Error] block passed check with invalid hash")
	assert.EqualError(t, err, "[PowCheckBlockSanity] block check aux pow failed")
}

func TestCheckCoinbaseArbitratorsReward(t *testing.T) {
	arbitratorsStr := []string{
		"023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a",
		"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9",
		"0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7",
		"03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd",
		"0393e823c2087ed30871cbea9fa5121fa932550821e9f3b17acef0e581971efab0",
	}
	candidatesStr := []string{
		"03e333657c788a20577c0288559bd489ee65514748d18cb1dc7560ae4ce3d45613",
		"02dd22722c3b3a284929e4859b07e6a706595066ddd2a0b38e5837403718fb047c",
		"03e4473b918b499e4112d281d805fc8d8ae7ac0a71ff938cba78006bf12dd90a85",
		"03dd66833d28bac530ca80af0efbfc2ec43b4b87504a41ab4946702254e7f48961",
		"02c8a87c076112a1b344633184673cfb0bb6bce1aca28c78986a7b1047d257a448",
	}

	arbitrators := make([]state.ArbiterMember, 0)
	candidates := make([]state.ArbiterMember, 0)
	arbitratorHashes := make([]*common.Uint168, 0)
	candidateHashes := make([]*common.Uint168, 0)
	ownerVotes := make(map[common.Uint168]common.Fixed64)
	totalVotesInRound := 0

	for i, v := range arbitratorsStr {
		vote := i + 10
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		arbitrators = append(arbitrators, ar)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		arbitratorHashes = append(arbitratorHashes, hash)
		ownerVotes[*hash] = common.Fixed64(vote)
		totalVotesInRound += vote
	}
	fmt.Println()
	for i, v := range candidatesStr {
		vote := i + 1
		a, _ := common.HexStringToBytes(v)
		ar, _ := state.NewOriginArbiter(state.Origin, a)
		arbitrators = append(arbitrators, ar)
		hash, _ := contract.PublicKeyToStandardProgramHash(a)
		candidateHashes = append(candidateHashes, hash)
		ownerVotes[*hash] = common.Fixed64(vote)
		totalVotesInRound += vote
	}

	originLedger := DefaultLedger
	arbitratorsMock := &state.ArbitratorsMock{
		CurrentArbitrators:          arbitrators,
		CurrentCandidates:           candidates,
		CurrentOwnerProgramHashes:   arbitratorHashes,
		CandidateOwnerProgramHashes: candidateHashes,
		OwnerVotesInRound:           ownerVotes,
		TotalVotesInRound:           common.Fixed64(totalVotesInRound),
		ArbitersRoundReward:         map[common.Uint168]common.Fixed64{},
	}
	DefaultLedger = &Ledger{
		Arbitrators: arbitratorsMock,
	}
	DefaultLedger.Arbitrators = arbitratorsMock

	rewardInCoinbase := common.Fixed64(1000)
	dposTotalReward := float64(rewardInCoinbase) * 0.35
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := dposTotalReward - totalBlockConfirmReward
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(5)))
	rewardPerVote := totalTopProducersReward / float64(totalVotesInRound)
	tx := &types.Transaction{
		Version: 0,
		TxType:  types.CoinBase,
	}
	tx.Outputs = []*types.Output{
		{ProgramHash: FoundationAddress, Value: common.Fixed64(float64(rewardInCoinbase) * 0.30)},
		{ProgramHash: common.Uint168{}, Value: common.Fixed64(float64(rewardInCoinbase) * 0.35)},
	}

	for _, v := range arbitratorHashes {
		vote := ownerVotes[*v]
		individualProducerReward := common.Fixed64(rewardPerVote * float64(vote))
		arbitratorsMock.ArbitersRoundReward[*v] = individualBlockConfirmReward + individualProducerReward
		tx.Outputs = append(tx.Outputs, &types.Output{ProgramHash: *v, Value: individualBlockConfirmReward + individualProducerReward})
	}
	for _, v := range candidateHashes {
		vote := ownerVotes[*v]
		individualProducerReward := common.Fixed64(rewardPerVote * float64(vote))
		arbitratorsMock.ArbitersRoundReward[*v] = individualProducerReward
		tx.Outputs = append(tx.Outputs, &types.Output{ProgramHash: *v, Value: individualProducerReward})
	}
	assert.NoError(t, checkCoinbaseArbitratorsReward(tx))

	DefaultLedger = originLedger
}

func TestCRDuplicateTx(t *testing.T) {
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	code := getValideCode(publicKeyStr1)
	nickname := randomString()
	cidPointer := getValideCID(publicKeyStr1)
	cid := *cidPointer

	TestRegisterCR := func(t *testing.T) {
		OneRegisterCRTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			registerCRTxPointer := generateRegisterCR(code, cid, nickname)
			block.Transactions = append(block.Transactions, registerCRTxPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneRegisterCRTest()

		TwoRegisterCRTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			registerCRTxPointer := generateRegisterCR(code, cid, nickname)
			block.Transactions = append(block.Transactions, registerCRTxPointer)
			block.Transactions = append(block.Transactions, registerCRTxPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoRegisterCRTest()
	}
	TestRegisterCR(t)

	TestUpdateCR := func(t *testing.T) {
		OneUpdateCRTest := func(t *testing.T) {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			updateCRPointer := generateUpdateCR(code, cid, nickname)
			block.Transactions = append(block.Transactions, updateCRPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneUpdateCRTest(t)

		TwoUpdateCRTest := func(t *testing.T) {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			updateCRPointer := generateUpdateCR(code, cid, nickname)
			block.Transactions = append(block.Transactions, updateCRPointer)
			block.Transactions = append(block.Transactions, updateCRPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoUpdateCRTest(t)
	}
	TestUpdateCR(t)

	TestUnregisterCR := func(t *testing.T) {
		OneUnregisterCR := func(t *testing.T) {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			unregisterCRPointer := generateUnregisterCR(code)
			block.Transactions = append(block.Transactions, unregisterCRPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneUnregisterCR(t)

		TwoUnregisterCR := func(t *testing.T) {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			unregisterCRPointer := generateUnregisterCR(code)
			block.Transactions = append(block.Transactions, unregisterCRPointer)
			block.Transactions = append(block.Transactions, unregisterCRPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoUnregisterCR(t)
	}
	TestUnregisterCR(t)

	OneRegisterOneUpdate := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		registerCRTxPointer := generateRegisterCR(code, cid, nickname)
		updateCRPointer := generateUpdateCR(code, cid, nickname)
		block.Transactions = append(block.Transactions, registerCRTxPointer)
		block.Transactions = append(block.Transactions, updateCRPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneRegisterOneUpdate(t)

	OneRegisterOneUnregister := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		registerCRTxPointer := generateRegisterCR(code, cid, nickname)
		unregisterCRPointer := generateUnregisterCR(code)
		block.Transactions = append(block.Transactions, registerCRTxPointer)
		block.Transactions = append(block.Transactions, unregisterCRPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneRegisterOneUnregister(t)
	OneUpdateOneUnregister := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		updateCRPointer := generateUpdateCR(code, cid, nickname)
		unregisterCRPointer := generateUnregisterCR(code)
		block.Transactions = append(block.Transactions, updateCRPointer)
		block.Transactions = append(block.Transactions, unregisterCRPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneUpdateOneUnregister(t)

}

func TestProducerDuplicateTx(t *testing.T) {
	TestRegisterProducer := func(t *testing.T) {
		OneRegisterProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			registerProducerTxPointer := generateRegisterProducer()
			block.Transactions = append(block.Transactions, registerProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneRegisterProducerTest()

		TwoRegisterProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			registerProducerTxPointer := generateRegisterProducer()
			block.Transactions = append(block.Transactions, registerProducerTxPointer)
			block.Transactions = append(block.Transactions, registerProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoRegisterProducerTest()
	}
	TestRegisterProducer(t)
	TestUpdateProducer := func(t *testing.T) {
		OneUpdateProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			updateProducerTxPointer := generateUpdateProducer()
			block.Transactions = append(block.Transactions, updateProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneUpdateProducerTest()

		TwoUpdateProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			updateProducerTxPointer := generateUpdateProducer()
			block.Transactions = append(block.Transactions, updateProducerTxPointer)
			block.Transactions = append(block.Transactions, updateProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoUpdateProducerTest()
	}
	TestUpdateProducer(t)
	TestCancelProducer := func(t *testing.T) {
		OneCancelProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			cancelProducerTxPointer := generateCancelProducer()
			block.Transactions = append(block.Transactions, cancelProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.NoError(t, err)
		}
		OneCancelProducerTest()

		TwoCancelProducerTest := func() {
			var block types.Block
			block.Transactions = make([]*types.Transaction, 0)
			cancelProducerTxPointer := generateCancelProducer()
			block.Transactions = append(block.Transactions, cancelProducerTxPointer)
			block.Transactions = append(block.Transactions, cancelProducerTxPointer)
			err := checkDuplicateTx(&block)
			assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
		}
		TwoCancelProducerTest()
	}
	TestCancelProducer(t)

	OneRegisterOneUpdate := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		registerProducerTxPointer := generateRegisterProducer()
		updateProducerTxPointer := generateUpdateProducer()
		block.Transactions = append(block.Transactions, registerProducerTxPointer)
		block.Transactions = append(block.Transactions, updateProducerTxPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneRegisterOneUpdate(t)

	OneRegisterOneCancel := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		registerProducerTxPointer := generateRegisterProducer()
		cancelProducerTxPointer := generateCancelProducer()
		block.Transactions = append(block.Transactions, registerProducerTxPointer)
		block.Transactions = append(block.Transactions, cancelProducerTxPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneRegisterOneCancel(t)
	OneUpdateOneCancel := func(t *testing.T) {
		var block types.Block
		block.Transactions = make([]*types.Transaction, 0)
		updateProducerTxPointer := generateUpdateProducer()
		cancelProducerTxPointer := generateCancelProducer()
		block.Transactions = append(block.Transactions, updateProducerTxPointer)
		block.Transactions = append(block.Transactions, cancelProducerTxPointer)
		err := checkDuplicateTx(&block)
		assert.Error(t, err, "[PowCheckBlockSanity] block contains duplicate CR")
	}
	OneUpdateOneCancel(t)
}

func generateRegisterProducer() *types.Transaction {
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	return &types.Transaction{
		TxType: types.RegisterProducer,
		Payload: &payload.ProducerInfo{
			OwnerPublicKey: publicKey1,
			NodePublicKey:  publicKey1,
			NickName:       "nickname 1",
			Url:            "http://www.elastos_test.com",
			Location:       1,
			NetAddress:     "127.0.0.1:20338",
		},
	}
}

func generateUpdateProducer() *types.Transaction {
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	return &types.Transaction{
		TxType: types.UpdateProducer,
		Payload: &payload.ProducerInfo{
			OwnerPublicKey: publicKey1,
			NodePublicKey:  publicKey1,
			NickName:       "nickname 1",
			Url:            "http://www.elastos_test.com",
			Location:       1,
			NetAddress:     "127.0.0.1:20338",
		},
	}
}

func generateCancelProducer() *types.Transaction {
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	return &types.Transaction{
		TxType: types.CancelProducer,
		Payload: &payload.ProcessProducer{
			OwnerPublicKey: publicKey1,
		},
	}
}

func generateRegisterCR(code []byte, cid common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.RegisterCR,
		Payload: &payload.CRInfo{
			Code:     code,
			CID:      cid,
			NickName: nickname,
		},
	}
}

func generateUpdateCR(code []byte, cid common.Uint168,
	nickname string) *types.Transaction {
	return &types.Transaction{
		TxType: types.UpdateCR,
		Payload: &payload.CRInfo{
			Code:     code,
			CID:      cid,
			NickName: nickname,
		},
	}
}

func generateUnregisterCR(code []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.UnregisterCR,
		Payload: &payload.UnregisterCR{
			CID: *getCID(code),
		},
	}
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}

func randomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}

func randomUint168() *common.Uint168 {
	randBytes := make([]byte, 21)
	rand.Read(randBytes)
	result, _ := common.Uint168FromBytes(randBytes)

	return result
}

func getValideCode(publicKeyStr string) []byte {
	publicKey1, _ := common.HexStringToBytes(publicKeyStr)
	pk1, _ := crypto.DecodePoint(publicKey1)
	ct1, _ := contract.CreateStandardContract(pk1)
	return ct1.Code
}

func getValideCID(publicKeyStr string) *common.Uint168 {
	code := getValideCode(publicKeyStr)
	return getCID(code)
}
