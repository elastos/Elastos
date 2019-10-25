// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"crypto/elliptic"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"math"
	mrand "math/rand"
	"path/filepath"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/utils/test"

	"github.com/stretchr/testify/suite"
)

type txValidatorTestSuite struct {
	suite.Suite

	ELA               int64
	foundationAddress common.Uint168
	HeightVersion1    uint32
	Chain             *BlockChain
	OriginalLedger    *Ledger
}

func (s *txValidatorTestSuite) SetupSuite() {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	params := &config.DefaultParams
	FoundationAddress = params.Foundation
	s.foundationAddress = params.Foundation

	chainStore, err := NewChainStore(
		filepath.Join(test.DataPath, "txvalidator"),
		params.GenesisBlock)
	if err != nil {
		s.Error(err)
	}
	s.Chain, err = New(chainStore, params,
		state.NewState(params, nil, nil),
		crstate.NewCommittee(params))
	if err != nil {
		s.Error(err)
	}

	s.OriginalLedger = DefaultLedger

	arbiters, err := state.NewArbitrators(params,
		nil, nil)
	if err != nil {
		s.Fail("initialize arbitrator failed")
	}
	arbiters.RegisterFunction(chainStore.GetHeight,
		func(height uint32) (*types.Block, error) {
			hash, err := chainStore.GetBlockHash(height)
			if err != nil {
				return nil, err
			}
			return chainStore.GetBlock(hash)
		})
	DefaultLedger = &Ledger{Arbitrators: arbiters}
}

func (s *txValidatorTestSuite) TearDownSuite() {
	s.Chain.db.Close()
	DefaultLedger = s.OriginalLedger
}

func (s *txValidatorTestSuite) TestCheckTxHeightVersion() {
	// set blockHeight1 less than CRVotingStartHeight and set blockHeight2
	// to CRVotingStartHeight.
	blockHeight1 := s.Chain.chainParams.CRVotingStartHeight - 1
	blockHeight2 := s.Chain.chainParams.CRVotingStartHeight

	// check height version of registerCR transaction.
	registerCR := &types.Transaction{TxType: types.RegisterCR}
	err := s.Chain.checkTxHeightVersion(registerCR, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(registerCR, blockHeight2)
	s.NoError(err)

	// check height version of updateCR transaction.
	updateCR := &types.Transaction{TxType: types.UpdateCR}
	err = s.Chain.checkTxHeightVersion(updateCR, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(updateCR, blockHeight2)
	s.NoError(err)

	// check height version of unregister transaction.
	unregisterCR := &types.Transaction{TxType: types.UnregisterCR}
	err = s.Chain.checkTxHeightVersion(unregisterCR, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(unregisterCR, blockHeight2)
	s.NoError(err)

	// check height version of unregister transaction.
	returnCoin := &types.Transaction{TxType: types.ReturnCRDepositCoin}
	err = s.Chain.checkTxHeightVersion(returnCoin, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(returnCoin, blockHeight2)
	s.NoError(err)

	// check height version of vote CR.
	voteCR := &types.Transaction{
		Version: 0x09,
		TxType:  types.TransferAsset,
		Outputs: []*types.Output{
			{
				AssetID:     common.Uint256{},
				Value:       0,
				OutputLock:  0,
				ProgramHash: common.Uint168{},
				Type:        types.OTVote,
				Payload: &outputpayload.VoteOutput{
					Version: outputpayload.VoteProducerAndCRVersion,
				},
			},
		},
	}
	err = s.Chain.checkTxHeightVersion(voteCR, blockHeight1)
	s.EqualError(err, "not support VoteProducerAndCRVersion "+
		"before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(voteCR, blockHeight2)
	s.NoError(err)
}

func (s *txValidatorTestSuite) TestCheckTransactionSize() {
	tx := buildTx()
	buf := new(bytes.Buffer)
	err := tx.Serialize(buf)
	if !s.NoError(err) {
		return
	}

	// normal
	err = checkTransactionSize(tx)
	s.NoError(err, "[CheckTransactionSize] passed normal size")
}

func (s *txValidatorTestSuite) TestCheckTransactionInput() {
	// coinbase transaction
	tx := newCoinBaseTransaction(new(payload.CoinBase), 0)
	err := checkTransactionInput(tx)
	s.NoError(err)

	// invalid coinbase refer index
	tx.Inputs[0].Previous.Index = 0
	err = checkTransactionInput(tx)
	s.EqualError(err, "invalid coinbase input")

	// invalid coinbase refer id
	tx.Inputs[0].Previous.Index = math.MaxUint16
	rand.Read(tx.Inputs[0].Previous.TxID[:])
	err = checkTransactionInput(tx)
	s.EqualError(err, "invalid coinbase input")

	// multiple coinbase inputs
	tx.Inputs = append(tx.Inputs, &types.Input{})
	err = checkTransactionInput(tx)
	s.EqualError(err, "coinbase must has only one input")

	// normal transaction
	tx = buildTx()
	err = checkTransactionInput(tx)
	s.NoError(err)

	// no inputs
	tx.Inputs = nil
	err = checkTransactionInput(tx)
	s.EqualError(err, "transaction has no inputs")

	// normal transaction with coinbase input
	tx.Inputs = append(tx.Inputs, &types.Input{Previous: *types.NewOutPoint(common.EmptyHash, math.MaxUint16)})
	err = checkTransactionInput(tx)
	s.EqualError(err, "invalid transaction input")

	// duplicated inputs
	tx = buildTx()
	tx.Inputs = append(tx.Inputs, tx.Inputs[0])
	err = checkTransactionInput(tx)
	s.EqualError(err, "duplicated transaction inputs")
}

func (s *txValidatorTestSuite) TestCheckTransactionOutput() {
	// coinbase
	tx := newCoinBaseTransaction(new(payload.CoinBase), 0)
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress},
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress},
	}
	err := s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "Asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30% (CheckTxOut version)
	totalReward := config.DefaultParams.RewardPerBlock
	fmt.Printf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// reward to foundation in coinbase < 30% (CheckTxOut version)
	foundationReward = common.Fixed64(float64(totalReward) * 0.299999)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 1
	tx.Outputs = nil
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "asset ID in output is invalid")

	// should only have one special output
	tx.Version = types.TxVersion09
	tx.Outputs = []*types.Output{}
	address := common.Uint168{}
	address[0] = byte(contract.PrefixStandard)
	appendSpecial := func() []*types.Output {
		return append(tx.Outputs, &types.Output{
			Type:        types.OTVote,
			AssetID:     config.ELAAssetID,
			ProgramHash: address,
			Value:       common.Fixed64(mrand.Int63()),
			OutputLock:  mrand.Uint32(),
			Payload: &outputpayload.VoteOutput{
				Contents: []outputpayload.VoteContent{},
			},
		})
	}
	tx.Outputs = appendSpecial()
	s.NoError(s.Chain.checkTransactionOutput(s.HeightVersion1, tx))
	tx.Outputs = appendSpecial() // add another special output here
	originHeight := config.DefaultParams.PublicDPOSHeight
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "special output count should less equal than 1")

	// invalid program hash
	tx.Version = types.TxVersionDefault
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		address := common.Uint168{}
		address[0] = 0x23
		output.ProgramHash = address
	}
	config.DefaultParams.PublicDPOSHeight = 0
	s.NoError(s.Chain.checkTransactionOutput(s.HeightVersion1, tx))
	config.DefaultParams.PublicDPOSHeight = originHeight

	// new sideChainPow
	tx = &types.Transaction{
		TxType: 0x05,
		Outputs: []*types.Output{
			{
				Value: 0,
				Type:  0,
			},
		},
	}
	s.NoError(s.Chain.checkTransactionOutput(s.HeightVersion1, tx))

	tx.Outputs = []*types.Output{
		{
			Value: 0,
			Type:  0,
		},
		{
			Value: 0,
			Type:  0,
		},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "new sideChainPow tx must have only one output")

	tx.Outputs = []*types.Output{
		{
			Value: 100,
			Type:  0,
		},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "the value of new sideChainPow tx output must be 0")

	tx.Outputs = []*types.Output{
		{
			Value: 0,
			Type:  1,
		},
	}
	err = s.Chain.checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "the type of new sideChainPow tx output must be OTNone")
}

func (s *txValidatorTestSuite) TestCheckAmountPrecision() {
	// precision check
	for i := 8; i >= 0; i-- {
		amount := common.Fixed64(math.Pow(10, float64(i)))
		fmt.Printf("Amount %s", amount.String())
		s.Equal(true, checkAmountPrecise(amount, byte(8-i)))
		s.Equal(false, checkAmountPrecise(amount, byte(8-i-1)))
	}
}

func (s *txValidatorTestSuite) TestCheckAttributeProgram() {
	// valid attributes
	tx := buildTx()
	usages := []types.AttributeUsage{
		types.Nonce,
		types.Script,
		types.Description,
		types.DescriptionUrl,
		types.Memo,
	}
	for _, usage := range usages {
		attr := types.NewAttribute(usage, nil)
		tx.Attributes = append(tx.Attributes, &attr)
	}
	err := s.Chain.checkAttributeProgram(tx, 0)
	s.EqualError(err, "no programs found in transaction")

	// invalid attributes
	getInvalidUsage := func() types.AttributeUsage {
		var usage = make([]byte, 1)
	NEXT:
		rand.Read(usage)
		if types.IsValidAttributeType(types.AttributeUsage(usage[0])) {
			goto NEXT
		}
		return types.AttributeUsage(usage[0])
	}
	for i := 0; i < 10; i++ {
		attr := types.NewAttribute(getInvalidUsage(), nil)
		tx.Attributes = []*types.Attribute{&attr}
		err := s.Chain.checkAttributeProgram(tx, 0)
		s.EqualError(err, fmt.Sprintf("invalid attribute usage %v", attr.Usage))
	}
	tx.Attributes = nil

	// empty programs
	tx.Programs = []*program.Program{}
	err = s.Chain.checkAttributeProgram(tx, 0)
	s.EqualError(err, "no programs found in transaction")

	// nil program code
	p := &program.Program{}
	tx.Programs = append(tx.Programs, p)
	err = s.Chain.checkAttributeProgram(tx, 0)
	s.EqualError(err, "invalid program code nil")

	// nil program parameter
	var code = make([]byte, 21)
	rand.Read(code)
	p = &program.Program{Code: code}
	tx.Programs = []*program.Program{p}
	err = s.Chain.checkAttributeProgram(tx, 0)
	s.EqualError(err, "invalid program parameter nil")
}

func (s *txValidatorTestSuite) TestCheckTransactionPayload() {
	// normal
	tx := new(types.Transaction)
	payload := &payload.RegisterAsset{
		Asset: payload.Asset{
			Name:      "ELA",
			Precision: 0x08,
			AssetType: payload.Token,
		},
		Amount: 3300 * 10000 * 10000000,
	}
	tx.Payload = payload
	err := checkTransactionPayload(tx)
	s.NoError(err)

	// invalid precision
	payload.Asset.Precision = 9
	err = checkTransactionPayload(tx)
	s.EqualError(err, "Invalide asset Precision.")

	// invalid amount
	payload.Asset.Precision = 0
	payload.Amount = 1234567
	err = checkTransactionPayload(tx)
	s.EqualError(err, "Invalide asset value,out of precise.")
}

func (s *txValidatorTestSuite) TestCheckDuplicateSidechainTx() {
	hashStr1 := "8a6cb4b5ff1a4f8368c6513a536c663381e3fdeff738e9b437bd8fce3fb30b62"
	hashBytes1, _ := common.HexStringToBytes(hashStr1)
	hash1, _ := common.Uint256FromBytes(hashBytes1)
	hashStr2 := "cc62e14f5f9526b7f4ff9d34dcd0643dacb7886707c57f49ec97b95ec5c4edac"
	hashBytes2, _ := common.HexStringToBytes(hashStr2)
	hash2, _ := common.Uint256FromBytes(hashBytes2)

	// 1. Generate the ill withdraw transaction which have duplicate sidechain tx
	txn := new(types.Transaction)
	txn.TxType = types.WithdrawFromSideChain
	txn.Payload = &payload.WithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash1,
			*hash2,
			*hash1, // duplicate tx hash
		},
	}

	// 2. Run CheckDuplicateSidechainTx
	err := checkDuplicateSidechainTx(txn)
	s.EqualError(err, "Duplicate sidechain tx detected in a transaction")
}

func (s *txValidatorTestSuite) TestCheckTransactionBalance() {
	// WithdrawFromSideChain will pass check in any condition
	tx := new(types.Transaction)
	tx.TxType = types.WithdrawFromSideChain

	// single output

	outputValue1 := common.Fixed64(100 * s.ELA)
	deposit := newCoinBaseTransaction(new(payload.CoinBase), 0)
	deposit.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: outputValue1},
	}

	references := map[*types.Input]*types.Output{
		&types.Input{}: {
			Value: outputValue1,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {
			Value: outputValue1 + s.Chain.chainParams.MinTransactionFee,
		},
	}
	s.NoError(s.Chain.checkTransactionFee(tx, references))

	// multiple output

	outputValue1 = common.Fixed64(30 * s.ELA)
	outputValue2 := common.Fixed64(70 * s.ELA)
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: outputValue1},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: outputValue2},
	}

	references = map[*types.Input]*types.Output{
		&types.Input{}: {
			Value: outputValue1 + outputValue2,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {
			Value: outputValue1 + outputValue2 + s.Chain.chainParams.MinTransactionFee,
		},
	}
	s.NoError(s.Chain.checkTransactionFee(tx, references))
}

func (s *txValidatorTestSuite) TestCheckSideChainPowConsensus() {
	// 1. Generate a side chain pow transaction
	txn := new(types.Transaction)
	txn.TxType = types.SideChainPow
	txn.Payload = &payload.SideChainPow{
		SideBlockHash:   common.Uint256{1, 1, 1},
		SideGenesisHash: common.Uint256{2, 2, 2},
		BlockHeight:     uint32(10),
	}

	//2. Get arbitrator
	password1 := "1234"
	privateKey1, _ := common.HexStringToBytes(password1)
	publicKey := new(crypto.PublicKey)
	publicKey.X, publicKey.Y = elliptic.P256().ScalarBaseMult(privateKey1)
	arbitrator1, _ := publicKey.EncodePoint(true)

	password2 := "5678"
	privateKey2, _ := common.HexStringToBytes(password2)
	publicKey2 := new(crypto.PublicKey)
	publicKey2.X, publicKey2.Y = elliptic.P256().ScalarBaseMult(privateKey2)
	arbitrator2, _ := publicKey2.EncodePoint(true)

	//3. Sign transaction by arbitrator1
	buf := new(bytes.Buffer)
	txn.Payload.Serialize(buf, payload.SideChainPowVersion)
	signature, _ := crypto.Sign(privateKey1, buf.Bytes()[0:68])
	txn.Payload.(*payload.SideChainPow).Signature = signature

	//4. Run CheckSideChainPowConsensus
	s.NoError(CheckSideChainPowConsensus(txn, arbitrator1), "TestCheckSideChainPowConsensus failed.")

	s.Error(CheckSideChainPowConsensus(txn, arbitrator2), "TestCheckSideChainPowConsensus failed.")
}

func (s *txValidatorTestSuite) TestCheckDestructionAddress() {
	destructionAddress := "ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr"
	txID, _ := common.Uint256FromHexString("7e8863a503e90e6464529feb1c25d98c903e01bec00ccfea2475db4e37d7328b")
	programHash, _ := common.Uint168FromAddress(destructionAddress)
	reference := map[*types.Input]*types.Output{
		&types.Input{Previous: types.OutPoint{*txID, 1234}, Sequence: 123456}: {
			ProgramHash: *programHash,
		},
	}

	err := checkDestructionAddress(reference)
	s.EqualError(err, fmt.Sprintf("cannot use utxo from the destruction address"))
}

func (s *txValidatorTestSuite) TestCheckRegisterProducerTransaction() {
	// Generate a register producer transaction
	publicKeyStr1 := "02ca89a5fe6213da1b51046733529a84f0265abac59005f6c16f62330d20f02aeb"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKeyStr1 := "7a50d2b036d64fcb3d344cee429f61c4a3285a934c45582b26e8c9227bc1f33a"
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterProducer
	rpPayload := &payload.ProducerInfo{
		OwnerPublicKey: publicKey1,
		NodePublicKey:  publicKey1,
		NickName:       "nickname 1",
		Url:            "http://www.elastos_test.com",
		Location:       1,
		NetAddress:     "127.0.0.1:20338",
	}
	rpSignBuf := new(bytes.Buffer)
	err := rpPayload.SerializeUnsigned(rpSignBuf, payload.ProducerInfoVersion)
	s.NoError(err)
	rpSig, err := crypto.Sign(privateKey1, rpSignBuf.Bytes())
	s.NoError(err)
	rpPayload.Signature = rpSig
	txn.Payload = rpPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	publicKeyDeposit1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *publicKeyDeposit1,
	}}

	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.NoError(err)

	// Give an invalid owner public key in payload
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = errPublicKey
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "invalid owner public key in payload")

	// check node public when block height is higher than h2
	originHeight := config.DefaultParams.PublicDPOSHeight
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = errPublicKey
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkRegisterProducerTransaction(txn)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "invalid node public key in payload")

	// check node public key same with CRC
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = publicKey2
	pk, _ := common.HexStringToBytes(config.DefaultParams.CRCArbiters[0])
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = pk
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkRegisterProducerTransaction(txn)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "node public key can't equal with CRC")

	// check owner public key same with CRC
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = publicKey2
	pk, _ = common.HexStringToBytes(config.DefaultParams.CRCArbiters[0])
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = pk
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkRegisterProducerTransaction(txn)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "owner public key can't equal with CRC")

	// Invalidates the signature in payload
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = publicKey2
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = publicKey2
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "invalid signature in payload")

	// Give an invalid url in payload
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = publicKey1
	txn.Payload.(*payload.ProducerInfo).Url = ""
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "Field Url has invalid string length.")

	// Give a mismatching deposit address
	rpPayload.OwnerPublicKey = publicKey1
	rpPayload.Url = "www.test.com"
	rpSignBuf = new(bytes.Buffer)
	err = rpPayload.SerializeUnsigned(rpSignBuf, payload.ProducerInfoVersion)
	s.NoError(err)
	rpSig, err = crypto.Sign(privateKey1, rpSignBuf.Bytes())
	s.NoError(err)
	rpPayload.Signature = rpSig
	txn.Payload = rpPayload

	publicKeyDeposit2, _ := contract.PublicKeyToDepositProgramHash(publicKey2)
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *publicKeyDeposit2,
	}}
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "deposit address does not match the public key in payload")

	// Give a insufficient deposit coin
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       4000,
		OutputLock:  0,
		ProgramHash: *publicKeyDeposit1,
	}}
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "producer deposit amount is insufficient")

	// Multi deposit addresses
	txn.Outputs = []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       5000 * 100000000,
			OutputLock:  0,
			ProgramHash: *publicKeyDeposit1,
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       5000 * 100000000,
			OutputLock:  0,
			ProgramHash: *publicKeyDeposit1,
		}}
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "there must be only one deposit address in outputs")
}

func getCodeByPubKeyStr(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	return redeemScript
}
func getCodeHexStr(publicKey string) string {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	codeHexStr := common.BytesToHexString(redeemScript)
	return codeHexStr
}

func (s *txValidatorTestSuite) TestCheckVoteProducerOutput() {
	// 1. Generate a vote output v0
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	outputs1 := []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType:       outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
							{publicKey1, 0},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 2,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: 2,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
	}

	// 2. Check output payload v0
	err := outputs1[0].Payload.(*outputpayload.VoteOutput).Validate()
	s.NoError(err)

	err = outputs1[1].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid public key count")

	err = outputs1[2].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate candidate")

	err = outputs1[3].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid vote version")

	err = outputs1[4].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate vote type")

	err = outputs1[5].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid vote type")

	err = outputs1[6].Payload.(*outputpayload.VoteOutput).Validate()
	s.NoError(err)

	// 3. Generate a vote output v1
	outputs := []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType:       outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
							{publicKey1, 1},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 2,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
						},
					},
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: 2,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 1},
						},
					},
				},
			},
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: outputpayload.VoteProducerAndCRVersion,
				Contents: []outputpayload.VoteContent{
					outputpayload.VoteContent{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
	}

	// 2. Check output payload v1
	err = outputs[0].Payload.(*outputpayload.VoteOutput).Validate()
	s.NoError(err)

	err = outputs[1].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid public key count")

	err = outputs[2].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate candidate")

	err = outputs[3].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid vote version")

	err = outputs[4].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate vote type")

	err = outputs[5].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid vote type")

	err = outputs[6].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid candidate votes")
}

func (s *txValidatorTestSuite) TestCheckUpdateProducerTransaction() {
	publicKeyStr1 := "031e12374bae471aa09ad479f66c2306f4bcc4ca5b754609a82a1839b94b4721b9"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKeyStr1 := "94396a69462208b8fd96d83842855b867d3b0e663203cb31d0dfaec0362ec034"
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterProducer
	registerPayload := &payload.ProducerInfo{
		OwnerPublicKey: publicKey1,
		NodePublicKey:  publicKey1,
		NickName:       "",
		Url:            "",
		Location:       1,
		NetAddress:     "",
	}
	txn.Payload = registerPayload

	txn.Programs = []*program.Program{{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	block := &types.Block{
		Transactions: []*types.Transaction{
			txn,
		},
	}
	s.Chain.state.ProcessBlock(block, nil)

	txn.TxType = types.UpdateProducer
	updatePayload := &payload.ProducerInfo{
		OwnerPublicKey: publicKey1,
		NodePublicKey:  publicKey1,
		NickName:       "",
		Url:            "",
		Location:       2,
		NetAddress:     "",
	}
	txn.Payload = updatePayload
	s.Chain.state.ProcessBlock(block, nil)

	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "Field NickName has invalid string length.")

	updatePayload.NickName = "nick name"
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "Field Url has invalid string length.")

	updatePayload.Url = "www.elastos.org"
	updatePayload.OwnerPublicKey = errPublicKey
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "invalid owner public key in payload")

	// check node public when block height is higher than h2
	originHeight := config.DefaultParams.PublicDPOSHeight
	updatePayload.NodePublicKey = errPublicKey
	config.DefaultParams.PublicDPOSHeight = 0
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "invalid node public key in payload")
	config.DefaultParams.PublicDPOSHeight = originHeight

	// check node public key same with CRC
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = publicKey2
	pk, _ := common.HexStringToBytes(config.DefaultParams.CRCArbiters[0])
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = pk
	config.DefaultParams.PublicDPOSHeight = 0
	err := s.Chain.checkUpdateProducerTransaction(txn)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "node public key can't equal with CRC")

	// check owner public key same with CRC
	txn.Payload.(*payload.ProducerInfo).NodePublicKey = publicKey2
	pk, _ = common.HexStringToBytes(config.DefaultParams.CRCArbiters[0])
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = pk
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkUpdateProducerTransaction(txn)
	config.DefaultParams.PublicDPOSHeight = originHeight
	s.EqualError(err, "owner public key can't equal with CRC")

	updatePayload.OwnerPublicKey = publicKey2
	updatePayload.NodePublicKey = publicKey1
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "invalid signature in payload")

	updatePayload.OwnerPublicKey = publicKey1
	updateSignBuf := new(bytes.Buffer)
	err = updatePayload.SerializeUnsigned(updateSignBuf, payload.ProducerInfoVersion)
	s.NoError(err)
	updateSig, err := crypto.Sign(privateKey1, updateSignBuf.Bytes())
	s.NoError(err)
	updatePayload.Signature = updateSig
	s.NoError(s.Chain.checkUpdateProducerTransaction(txn))

	//rest of check test will be continued in chain test
}

func (s *txValidatorTestSuite) TestCheckCancelProducerTransaction() {
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.CancelProducer
	cancelPayload := &payload.ProcessProducer{
		OwnerPublicKey: publicKey1,
	}
	txn.Payload = cancelPayload

	txn.Programs = []*program.Program{{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	cancelPayload.OwnerPublicKey = errPublicKey
	s.EqualError(s.Chain.checkCancelProducerTransaction(txn), "invalid public key in payload")

	cancelPayload.OwnerPublicKey = publicKey2
	s.EqualError(s.Chain.checkCancelProducerTransaction(txn), "invalid signature in payload")
}

func (s *txValidatorTestSuite) TestCheckActivateProducerTransaction() {
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.ActivateProducer
	activatePayload := &payload.ActivateProducer{
		NodePublicKey: publicKey1,
	}
	txn.Payload = activatePayload

	txn.Programs = []*program.Program{{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	activatePayload.NodePublicKey = errPublicKey
	s.EqualError(s.Chain.checkActivateProducerTransaction(txn, 0),
		"invalid public key in payload")

	activatePayload.NodePublicKey = publicKey2
	s.EqualError(s.Chain.checkActivateProducerTransaction(txn, 0),
		"invalid signature in payload")
}

func (s *txValidatorTestSuite) TestCheckRegisterCRTransaction() {
	// Generate a register CR transaction

	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"
	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"
	nickName1 := randomString()

	hash1, _ := getDepositAddress(publicKeyStr1)
	hash2, _ := getDepositAddress(publicKeyStr2)

	txn := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)

	code1 := getCodeByPubKeyStr(publicKeyStr1)
	code2 := getCodeByPubKeyStr(publicKeyStr2)
	codeStr1 := common.BytesToHexString(code1)

	did1 := getDid(code1)
	did2 := getDid(code2)

	votingHeight := config.DefaultParams.CRVotingStartHeight

	// All ok
	err := s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	// Invalid payload
	txnUpdateCr := s.getUnregisterCRTx(publicKeyStr1, privateKeyStr1)
	err = s.Chain.checkRegisterCRTransaction(txnUpdateCr, votingHeight)
	s.EqualError(err, "invalid payload")

	// Give an invalid NickName length 0 in payload
	nickName := txn.Payload.(*payload.CRInfo).NickName
	txn.Payload.(*payload.CRInfo).NickName = ""
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "Field NickName has invalid string length.")

	// Give an invalid NickName length more than 100 in payload
	txn.Payload.(*payload.CRInfo).NickName = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "Field NickName has invalid string length.")

	// Give an invalid url length 0 in payload
	url := txn.Payload.(*payload.CRInfo).Url
	txn.Payload.(*payload.CRInfo).Url = ""
	txn.Payload.(*payload.CRInfo).NickName = nickName
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "Field Url has invalid string length.")

	// Give an invalid url length more than 100 in payload
	txn.Payload.(*payload.CRInfo).Url = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "Field Url has invalid string length.")

	// Not in vote Period lower
	txn.Payload.(*payload.CRInfo).Url = url
	err = s.Chain.checkRegisterCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	// Not in vote Period upper c.params.CRCommitteeStartHeight
	err = s.Chain.checkRegisterCRTransaction(txn, config.DefaultParams.CRCommitteeStartHeight+1)
	s.EqualError(err, "should create tx during voting period")

	// Nickname already in use
	s.Chain.crCommittee.GetState().Nicknames[nickName1] = struct{}{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "nick name "+nickName1+" already inuse")

	delete(s.Chain.crCommittee.GetState().Nicknames, nickName1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	err = s.Chain.checkRegisterCRTransaction(txn, 0)
	s.EqualError(err, "should create tx during voting period")

	delete(s.Chain.crCommittee.GetState().CodeDIDMap, codeStr1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	// DID already exist
	s.Chain.crCommittee.GetState().CodeDIDMap[codeStr1] = *did1
	s.Chain.crCommittee.GetState().ActivityCandidates[*did1] = &crstate.Candidate{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "did "+did1.String()+" already exist")
	delete(s.Chain.crCommittee.GetState().ActivityCandidates, *did1)

	// Give an invalid code in payload
	txn.Payload.(*payload.CRInfo).Code = []byte{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "code is nil")

	// Give an invalid DID in payload
	txn.Payload.(*payload.CRInfo).Code = code1
	txn.Payload.(*payload.CRInfo).DID = common.Uint168{1, 2, 3}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "invalid did address")

	// Give a mismatching code and DID in payload
	txn.Payload.(*payload.CRInfo).DID = *did2
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "invalid did address")

	// Invalidates the signature in payload
	txn.Payload.(*payload.CRInfo).DID = *did1
	signatature := txn.Payload.(*payload.CRInfo).Signature
	txn.Payload.(*payload.CRInfo).Signature = randomSignature()
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Signature = signatature
	s.EqualError(err, "[Validation], Verify failed.")

	// Give a mismatching deposit address
	outPuts := txn.Outputs
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *hash2,
		Payload:     new(outputpayload.DefaultOutput),
	}}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	txn.Outputs = outPuts
	s.EqualError(err, "deposit address does not match the code in payload")

	// Give a insufficient deposit coin
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       4000 * 100000000,
		OutputLock:  0,
		ProgramHash: *hash1,
		Payload:     new(outputpayload.DefaultOutput),
	}}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	txn.Outputs = outPuts
	s.EqualError(err, "CR deposit amount is insufficient")

	// Multi deposit addresses
	txn.Outputs = []*types.Output{
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       5000 * 100000000,
			OutputLock:  0,
			ProgramHash: *hash1,
			Payload:     new(outputpayload.DefaultOutput),
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       5000 * 100000000,
			OutputLock:  0,
			ProgramHash: *hash1,
			Payload:     new(outputpayload.DefaultOutput),
		}}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	txn.Outputs = outPuts
	s.EqualError(err, "there must be only one deposit address in outputs")

	// Check correct register CR transaction with multi sign code.
	txn = s.getMultiSigRegisterCRTx(
		[]string{publicKeyStr1, publicKeyStr2, publicKeyStr3},
		[]string{privateKeyStr1, privateKeyStr2, privateKeyStr3}, nickName1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "CR not support multi sign code")

	txn = s.getMultiSigRegisterCRTx(
		[]string{publicKeyStr1, publicKeyStr2, publicKeyStr3},
		[]string{privateKeyStr1, privateKeyStr2}, nickName1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "CR not support multi sign code")

	txn = s.getMultiSigRegisterCRTx(
		[]string{publicKeyStr1, publicKeyStr2, publicKeyStr3},
		[]string{privateKeyStr1}, nickName1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "CR not support multi sign code")
}

func getDepositAddress(publicKeyStr string) (*common.Uint168, error) {
	publicKey, _ := common.HexStringToBytes(publicKeyStr)
	hash, err := contract.PublicKeyToDepositProgramHash(publicKey)
	if err != nil {
		return nil, err
	}
	return hash, nil
}

func getDid(code []byte) *common.Uint168 {
	ct1, _ := contract.CreateCRDIDContractByCode(code)
	return ct1.ToProgramHash()
}

func (s *txValidatorTestSuite) getRegisterCRTx(publicKeyStr, privateKeyStr, nickName string) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)

	code1 := getCodeByPubKeyStr(publicKeyStr1)

	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	did1 := ct1.ToProgramHash()

	hash1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     code1,
		DID:      *did1,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	signBuf := new(bytes.Buffer)
	crInfoPayload.SerializeUnsigned(signBuf, payload.CRInfoVersion)
	rcSig1, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crInfoPayload.Signature = rcSig1
	txn.Payload = crInfoPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}

	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *hash1,
		Type:        0,
		Payload:     new(outputpayload.DefaultOutput),
	}}
	return txn
}

func (s *txValidatorTestSuite) getMultiSigRegisterCRTx(
	publicKeyStrs, privateKeyStrs []string, nickName string) *types.Transaction {

	var publicKeys []*crypto.PublicKey
	for _, publicKeyStr := range publicKeyStrs {
		publicKeyBytes, _ := hex.DecodeString(publicKeyStr)
		publicKey, _ := crypto.DecodePoint(publicKeyBytes)
		publicKeys = append(publicKeys, publicKey)
	}

	multiCode, _ := contract.CreateMultiSigRedeemScript(len(publicKeys)*2/3, publicKeys)

	ctDID, _ := contract.CreateCRDIDContractByCode(multiCode)
	did := ctDID.ToProgramHash()

	ctDeposit, _ := contract.CreateDepositContractByCode(multiCode)
	deposit := ctDeposit.ToProgramHash()

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     multiCode,
		DID:      *did,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}

	signBuf := new(bytes.Buffer)
	crInfoPayload.SerializeUnsigned(signBuf, payload.CRInfoVersion)
	for _, privateKeyStr := range privateKeyStrs {
		privateKeyBytes, _ := hex.DecodeString(privateKeyStr)
		sig, _ := crypto.Sign(privateKeyBytes, signBuf.Bytes())
		crInfoPayload.Signature = append(crInfoPayload.Signature, byte(len(sig)))
		crInfoPayload.Signature = append(crInfoPayload.Signature, sig...)
	}

	txn.Payload = crInfoPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      multiCode,
		Parameter: nil,
	}}
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000 * 100000000,
		OutputLock:  0,
		ProgramHash: *deposit,
		Type:        0,
		Payload:     new(outputpayload.DefaultOutput),
	}}
	return txn
}

func (s *txValidatorTestSuite) getUpdateCRTx(publicKeyStr, privateKeyStr, nickName string) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)
	code1 := getCodeByPubKeyStr(publicKeyStr1)
	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	did1 := ct1.ToProgramHash()

	txn := new(types.Transaction)
	txn.TxType = types.UpdateCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     code1,
		DID:      *did1,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	signBuf := new(bytes.Buffer)
	err := crInfoPayload.SerializeUnsigned(signBuf, payload.CRInfoVersion)
	s.NoError(err)
	rcSig1, err := crypto.Sign(privateKey1, signBuf.Bytes())
	s.NoError(err)
	crInfoPayload.Signature = rcSig1
	txn.Payload = crInfoPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) getUnregisterCRTx(publicKeyStr, privateKeyStr string) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)

	code1 := getCodeByPubKeyStr(publicKeyStr1)

	txn := new(types.Transaction)
	txn.TxType = types.UnregisterCR
	txn.Version = types.TxVersion09
	unregisterCRPayload := &payload.UnregisterCR{
		DID: *getDid(code1),
	}
	signBuf := new(bytes.Buffer)
	err := unregisterCRPayload.SerializeUnsigned(signBuf, payload.UnregisterCRVersion)
	s.NoError(err)
	rcSig1, err := crypto.Sign(privateKey1, signBuf.Bytes())
	s.NoError(err)
	unregisterCRPayload.Signature = rcSig1
	txn.Payload = unregisterCRPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr1),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) getCRMember(publicKeyStr, privateKeyStr, nickName string) *crstate.CRMember {
	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)
	code1 := getCodeByPubKeyStr(publicKeyStr1)
	ct1, _ := contract.CreateCRDIDContractByCode(code1)
	did1 := ct1.ToProgramHash()

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	crInfoPayload := payload.CRInfo{
		Code:     code1,
		DID:      *did1,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	signBuf := new(bytes.Buffer)
	crInfoPayload.SerializeUnsigned(signBuf, payload.CRInfoVersion)
	rcSig1, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crInfoPayload.Signature = rcSig1

	return &crstate.CRMember{
		Info: crInfoPayload,
	}
}

func (s *txValidatorTestSuite) getCRCProposalTx(publicKeyStr, privateKeyStr,
	crPublicKeyStr, crPrivateKeyStr string) *types.Transaction {

	publicKey1, _ := common.HexStringToBytes(publicKeyStr)
	privateKey1, _ := common.HexStringToBytes(privateKeyStr)

	privateKey2, _ := common.HexStringToBytes(crPrivateKeyStr)
	code2 := getCodeByPubKeyStr(crPublicKeyStr)

	draftData := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposal
	txn.Version = types.TxVersion09
	crcProposalPayload := &payload.CRCProposal{
		ProposalType:     payload.Normal,
		SponsorPublicKey: publicKey1,
		CRSponsorDID:     *getDid(code2),
		DraftHash:        common.Hash(draftData),
		DraftData:        draftData,
		Budgets:          []common.Fixed64{1, 1, 1},
	}

	signBuf := new(bytes.Buffer)
	crcProposalPayload.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalPayload.Sign = sig

	common.WriteVarBytes(signBuf, sig)
	crSig, _ := crypto.Sign(privateKey2, signBuf.Bytes())
	crcProposalPayload.CRSign = crSig

	txn.Payload = crcProposalPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) TestCheckCRCProposalTrackingTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"

	leaderPubKey, _ := common.HexStringToBytes(publicKeyStr1)

	proposalHash := randomUint256()
	recipient := randomUint168()
	votingHeight := config.DefaultParams.CRVotingStartHeight

	// Set secretary general.
	s.Chain.chainParams.SecretaryGeneral = publicKeyStr3

	// Check Common tracking tx.
	txn := s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:     0,
				SponsorPublicKey: leaderPubKey,
				CRSponsorDID:     *randomUint168(),
				DraftHash:        *randomUint256(),
				Budgets:          []common.Fixed64{100, 200, 300},
				Recipient:        *recipient,
			},
			CurrentStage:   1,
			Status:         crstate.VoterAgreed,
			ProposalLeader: leaderPubKey,
		}

	err := s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 1, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "appropriation need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewLeaderPubKey need to be empty")

	// Check Progress tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 1, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "invalid stage")

	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 1, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "appropriation need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 1, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewLeaderPubKey need to be empty")

	// Check Terminated tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 1, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 0, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "appropriation need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewLeaderPubKey need to be empty")

	// Check ProposalLeader tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.ProposalLeader, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)

	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.ProposalLeader, *proposalHash, 1, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.ProposalLeader, *proposalHash, 0, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "appropriation need to be zero")

	txn = s.getCRCProposalTrackingTx(payload.ProposalLeader, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "invalid new proposal leader public key")

	// Check Appropriation tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.Appropriation, *proposalHash, 2, 200,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Appropriation, *proposalHash, 0, 200,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "invalid stage")

	txn = s.getCRCProposalTrackingTx(payload.Appropriation, *proposalHash, 2, 300,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "invalid appropriation")

	txn = s.getCRCProposalTrackingTx(payload.Appropriation, *proposalHash, 2, 200,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewLeaderPubKey need to be empty")

	// Check invalid proposal hash.
	txn = s.getCRCProposalTrackingTx(payload.Common, *randomUint256(), 0, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "proposal not exist")

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	// Check proposal status is not VoterAgreed.
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:     0,
				SponsorPublicKey: leaderPubKey,
				CRSponsorDID:     *randomUint168(),
				DraftHash:        *randomUint256(),
				Budgets:          []common.Fixed64{100, 200, 300},
				Recipient:        *recipient,
			},
			CurrentStage:     1,
			TerminatedHeight: 100,
			Status:           crstate.VoterCanceled,
			ProposalLeader:   leaderPubKey,
		}
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash].TerminatedHeight = 100
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "proposal status is not VoterAgreed")

	// Check reach max proposal tracking count.
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:     0,
				SponsorPublicKey: leaderPubKey,
				CRSponsorDID:     *randomUint168(),
				DraftHash:        *randomUint256(),
				Budgets:          []common.Fixed64{100, 200, 300},
				Recipient:        *recipient,
			},
			CurrentStage:   1,
			TrackingCount:  128,
			Status:         crstate.VoterAgreed,
			ProposalLeader: leaderPubKey,
		}
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "reached max tracking count")

	// Check draft data of proposal tracking document.
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:     0,
				SponsorPublicKey: leaderPubKey,
				CRSponsorDID:     *randomUint168(),
				DraftHash:        *randomUint256(),
				DraftData:        randomBytes(10),
				Budgets:          []common.Fixed64{100, 200, 300},
				Recipient:        *recipient,
			},
			CurrentStage:   1,
			TrackingCount:  1,
			Status:         crstate.VoterAgreed,
			ProposalLeader: leaderPubKey,
		}
	txn.Payload.(*payload.CRCProposalTracking).DocumentData = randomBytes(7)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "failed to check document data hash")
}

func (s *txValidatorTestSuite) getCRCProposalTrackingTx(
	trackingType payload.CRCProposalTrackingType,
	proposalHash common.Uint256, stage uint8, appropriation common.Fixed64,
	leaderPublicKeyStr, leaderPrivateKeyStr,
	newLeaderPublicKeyStr, newLeaderPrivateKeyStr,
	sgPublicKeyStr, sgPrivateKeyStr string) *types.Transaction {

	leaderPublicKey, _ := common.HexStringToBytes(leaderPublicKeyStr)
	leaderPrivateKey, _ := common.HexStringToBytes(leaderPrivateKeyStr)

	newLeaderPublicKey, _ := common.HexStringToBytes(newLeaderPublicKeyStr)
	newLeaderPrivateKey, _ := common.HexStringToBytes(newLeaderPrivateKeyStr)

	sgPrivateKey, _ := common.HexStringToBytes(sgPrivateKeyStr)

	documentData := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalTracking
	txn.Version = types.TxVersion09
	cPayload := &payload.CRCProposalTracking{
		ProposalTrackingType: trackingType,
		ProposalHash:         proposalHash,
		Stage:                stage,
		DocumentHash:         common.Hash(documentData),
		DocumentData:         documentData,
		Appropriation:        appropriation,
		LeaderPubKey:         leaderPublicKey,
		NewLeaderPubKey:      newLeaderPublicKey,
	}

	signBuf := new(bytes.Buffer)
	cPayload.SerializeUnsigned(signBuf, payload.CRCProposalTrackingVersion)
	sig, _ := crypto.Sign(leaderPrivateKey, signBuf.Bytes())
	cPayload.LeaderSign = sig

	if newLeaderPublicKeyStr != "" && newLeaderPrivateKeyStr != "" {
		common.WriteVarBytes(signBuf, sig)
		crSig, _ := crypto.Sign(newLeaderPrivateKey, signBuf.Bytes())
		cPayload.NewLeaderSign = crSig
		sig = crSig
	}

	common.WriteVarBytes(signBuf, sig)
	crSig, _ := crypto.Sign(sgPrivateKey, signBuf.Bytes())
	cPayload.SecretaryGeneralSign = crSig

	txn.Payload = cPayload
	return txn
}

func (s *txValidatorTestSuite) TestCheckCRCAppropriationTransaction() {
	// Set CRC foundation and CRC committee address.
	s.Chain.chainParams.CRCFoundation = *randomUint168()
	s.Chain.chainParams.CRCCommitteeAddress = *randomUint168()

	// Set CRC foundation and CRC committee amount.
	s.Chain.crCommittee.CRCFoundationAmount = common.Fixed64(990 * 1e8)
	s.Chain.crCommittee.CRCCommitteeAmount = common.Fixed64(10 * 1e8)
	s.Chain.crCommittee.CRCCommitteeUsedAmount = common.Fixed64(0 * 1e8)

	// Create reference.
	reference := make(map[*types.Input]*types.Output)
	input := &types.Input{
		Previous: types.OutPoint{
			TxID:  *randomUint256(),
			Index: 0,
		},
	}
	refOutput := &types.Output{
		Value:       990 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCFoundation,
	}
	refOutputErr := &types.Output{
		Value:       990 * 1e8,
		ProgramHash: *randomUint168(),
	}
	reference[input] = refOutput

	// Create CRC appropriation transaction.
	output1 := &types.Output{
		Value:       90 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCCommitteeAddress,
	}
	output2 := &types.Output{
		Value:       900 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCFoundation,
	}
	output1Err := &types.Output{
		Value:       91 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCCommitteeAddress,
	}
	output2Err := &types.Output{
		Value:       899 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCFoundation,
	}

	// Check correct transaction.
	s.Chain.crCommittee.NeedAppropriation = true
	txn := s.getCRCAppropriationTx(input, output1, output2)
	err := s.Chain.checkCRCAppropriationTransaction(txn, reference)
	s.NoError(err)

	// Appropriation transaction already exist.
	s.Chain.crCommittee.NeedAppropriation = false
	err = s.Chain.checkCRCAppropriationTransaction(txn, reference)
	s.EqualError(err, "should have no appropriation transaction")

	// Input does not from CRC foundation
	s.Chain.crCommittee.NeedAppropriation = true
	reference[input] = refOutputErr
	txn = s.getCRCAppropriationTx(input, output1, output2)
	err = s.Chain.checkCRCAppropriationTransaction(txn, reference)
	s.EqualError(err, "input does not from CRC foundation")

	// Inputs total amount does not equal to outputs total amount.
	reference[input] = refOutput
	txn = s.getCRCAppropriationTx(input, output1, output2Err)
	err = s.Chain.checkCRCAppropriationTransaction(txn, reference)
	s.EqualError(err, "inputs does not equal to outputs "+
		"amount, inputs:990 outputs:989")

	// Invalid CRC appropriation amount.
	txn = s.getCRCAppropriationTx(input, output1Err, output2Err)
	err = s.Chain.checkCRCAppropriationTransaction(txn, reference)
	s.EqualError(err, "invalid appropriation amount 91, need to be 90")
}

func (s *txValidatorTestSuite) getCRCAppropriationTx(input *types.Input,
	output1 *types.Output, output2 *types.Output) *types.Transaction {
	txn := new(types.Transaction)
	txn.TxType = types.CRCAppropriation
	txn.Version = types.TxVersion09
	cPayload := &payload.CRCAppropriation{}
	txn.Payload = cPayload
	txn.Inputs = []*types.Input{input}
	txn.Outputs = []*types.Output{output1, output2}

	return txn
}

func (s *txValidatorTestSuite) TestCrInfoSanityCheck() {
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)

	pk1, _ := crypto.DecodePoint(publicKey1)
	ct1, _ := contract.CreateStandardContract(pk1)
	hash1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)

	rcPayload := &payload.CRInfo{
		Code:     ct1.Code,
		DID:      *hash1,
		NickName: "nickname 1",
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}

	rcSignBuf := new(bytes.Buffer)
	err := rcPayload.SerializeUnsigned(rcSignBuf, payload.CRInfoVersion)
	s.NoError(err)

	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)
	rcSig1, err := crypto.Sign(privateKey1, rcSignBuf.Bytes())
	s.NoError(err)

	//test ok
	rcPayload.Signature = rcSig1
	err = s.Chain.crInfoSanityCheck(rcPayload)
	s.NoError(err)

	//invalid code
	rcPayload.Code = []byte{1, 2, 3, 4, 5}
	err = s.Chain.crInfoSanityCheck(rcPayload)
	s.EqualError(err, "invalid code")

	//todo CHECKMULTISIG
}

func (s *txValidatorTestSuite) TestCheckUpdateCRTransaction() {

	// Generate a UpdateCR CR transaction
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"

	nickName1 := "nickname 1"
	nickName2 := "nickname 2"
	nickName3 := "nickname 3"

	votingHeight := config.DefaultParams.CRVotingStartHeight
	//
	//registe an cr to update
	registerCRTxn1 := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	registerCRTxn2 := s.getRegisterCRTx(publicKeyStr2, privateKeyStr2, nickName2)

	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
		},
	}
	s.Chain.crCommittee.GetState().ProcessBlock(block, nil)

	//ok nothing wrong
	hash2, err := getDepositAddress(publicKeyStr2)
	txn := s.getUpdateCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	s.NoError(err)

	//invalid payload
	unregisterTx := s.getUnregisterCRTx(publicKeyStr1, privateKeyStr1)
	err = s.Chain.checkUpdateCRTransaction(unregisterTx, votingHeight)
	s.EqualError(err, "invalid payload")
	// Give an invalid NickName length 0 in payload
	nickName := txn.Payload.(*payload.CRInfo).NickName
	txn.Payload.(*payload.CRInfo).NickName = ""
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).NickName = nickName
	s.EqualError(err, "Field NickName has invalid string length.")

	// Give an invalid NickName length more than 100 in payload
	txn.Payload.(*payload.CRInfo).NickName = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).NickName = nickName
	s.EqualError(err, "Field NickName has invalid string length.")

	// Give an invalid url length 0 in payload
	url := txn.Payload.(*payload.CRInfo).Url
	txn.Payload.(*payload.CRInfo).Url = ""
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Url = url
	s.EqualError(err, "Field Url has invalid string length.")

	// Give an invalid url length more than 100 in payload
	txn.Payload.(*payload.CRInfo).Url = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Url = url
	s.EqualError(err, "Field Url has invalid string length.")

	// Give an invalid code in payload
	code := txn.Payload.(*payload.CRInfo).Code
	txn.Payload.(*payload.CRInfo).Code = []byte{1, 2, 3, 4, 5}
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Code = code
	s.EqualError(err, "invalid did address")

	// Give an invalid DID in payload
	did := txn.Payload.(*payload.CRInfo).DID
	txn.Payload.(*payload.CRInfo).DID = common.Uint168{1, 2, 3}
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).DID = did
	s.EqualError(err, "invalid did address")

	// Give a mismatching code and DID in payload
	txn.Payload.(*payload.CRInfo).DID = *hash2
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).DID = did
	s.EqualError(err, "invalid did address")

	// Invalidates the signature in payload
	signatur := txn.Payload.(*payload.CRInfo).Signature
	txn.Payload.(*payload.CRInfo).Signature = randomSignature()
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Signature = signatur
	s.EqualError(err, "[Validation], Verify failed.")

	//not in vote Period lower
	err = s.Chain.checkUpdateCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	//not in vote Period lower upper c.params.CRCommitteeStartHeight
	err = s.Chain.checkUpdateCRTransaction(txn, config.DefaultParams.CRCommitteeStartHeight+1)
	s.EqualError(err, "should create tx during voting period")

	//updating unknown CR
	txn3 := s.getUpdateCRTx(publicKeyStr3, privateKeyStr3, nickName3)
	err = s.Chain.checkUpdateCRTransaction(txn3, votingHeight)
	s.EqualError(err, "updating unknown CR")

	//nick name already exist
	txn1Copy := s.getUpdateCRTx(publicKeyStr1, privateKeyStr1, nickName2)
	err = s.Chain.checkUpdateCRTransaction(txn1Copy, votingHeight)
	str := fmt.Sprintf("nick name %s already exist", nickName2)
	s.EqualError(err, str)

}

func (s *txValidatorTestSuite) TestCheckUnregisterCRTransaction() {

	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	votingHeight := config.DefaultParams.CRVotingStartHeight
	nickName1 := "nickname 1"

	//register a cr to unregister
	registerCRTxn := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn,
		},
	}
	s.Chain.crCommittee.GetState().ProcessBlock(block, nil)
	//ok
	txn := s.getUnregisterCRTx(publicKeyStr1, privateKeyStr1)
	err := s.Chain.checkUnRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	//invalid payload need unregisterCR pass registerCr
	registerTx := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)
	err = s.Chain.checkUnRegisterCRTransaction(registerTx, votingHeight)
	s.EqualError(err, "invalid payload")

	//not in vote Period lower
	err = s.Chain.checkUnRegisterCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	//not in vote Period lower upper c.params.CRCommitteeStartHeight
	err = s.Chain.checkUnRegisterCRTransaction(txn, config.DefaultParams.CRCommitteeStartHeight+1)
	s.EqualError(err, "should create tx during voting period")

	//unregister unknown CR
	txn2 := s.getUnregisterCRTx(publicKeyStr2, privateKeyStr2)
	err = s.Chain.checkUnRegisterCRTransaction(txn2, votingHeight)
	s.EqualError(err, "unregister unknown CR")

	//wrong signature
	txn.Payload.(*payload.UnregisterCR).Signature = randomSignature()
	err = s.Chain.checkUnRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "[Validation], Verify failed.")
}

func (s *txValidatorTestSuite) getCRCProposalReviewTx(crPublicKeyStr,
	crPrivateKeyStr string) *types.Transaction {

	privateKey1, _ := common.HexStringToBytes(crPrivateKeyStr)
	code := getCodeByPubKeyStr(crPublicKeyStr)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalReview
	txn.Version = types.TxVersion09
	crcProposalReviewPayload := &payload.CRCProposalReview{
		ProposalHash: *randomUint256(),
		VoteResult:   payload.Approve,
		DID:          *getDid(code),
	}

	signBuf := new(bytes.Buffer)
	crcProposalReviewPayload.SerializeUnsigned(signBuf, payload.CRCProposalReviewVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalReviewPayload.Sign = sig

	txn.Payload = crcProposalReviewPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(crPublicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) TestCheckCRCProposalReviewTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	tenureHeight := config.DefaultParams.CRCommitteeStartHeight
	nickName1 := "nickname 1"

	fmt.Println("getcode ", getCodeHexStr("02e23f70b9b967af35571c32b1442d787c180753bbed5cd6e7d5a5cfe75c7fc1ff"))

	member1 := s.getCRMember(publicKeyStr1, privateKeyStr1, nickName1)
	s.Chain.crCommittee.Members[member1.Info.DID] = member1

	// ok
	txn := s.getCRCProposalReviewTx(publicKeyStr1, privateKeyStr1)
	crcProposalReview, _ := txn.Payload.(*payload.CRCProposalReview)
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalReview.ProposalHash] = nil
	err := s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.NoError(err)

	// invalid payload
	txn.Payload = &payload.CRInfo{}
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "invalid payload")

	// invalid content type
	txn = s.getCRCProposalReviewTx(publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposalReview).VoteResult = 0x10
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "VoteResult should be known")

	// proposal reviewer is not CR member
	txn = s.getCRCProposalReviewTx(publicKeyStr2, privateKeyStr2)
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "did correspond crMember not exists")

	delete(s.Chain.crCommittee.GetProposalManager().Proposals, crcProposalReview.ProposalHash)
	// invalid CR proposal reviewer signature
	txn = s.getCRCProposalReviewTx(publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposalReview).Sign = []byte{}
	crcProposalReview, _ = txn.Payload.(*payload.CRCProposalReview)
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalReview.ProposalHash] = nil
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "invalid signature length")
	delete(s.Chain.crCommittee.GetProposalManager().Proposals, crcProposalReview.ProposalHash)
}

func (s *txValidatorTestSuite) getCRCProposalWithdrawTx(crPublicKeyStr,
	crPrivateKeyStr string, stage uint8, recipient,
	commitee *common.Uint168, recipAmout, commiteAmout common.Fixed64) *types.
	Transaction {

	privateKey1, _ := common.HexStringToBytes(crPrivateKeyStr)
	pkBytes, _ := common.HexStringToBytes(crPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalWithdraw
	txn.Version = types.TxVersionDefault
	crcProposalWithdraw := &payload.CRCProposalWithdraw{
		ProposalHash:     *randomUint256(),
		SponsorPublicKey: pkBytes,
		Stage:            stage,
		Fee:              common.Fixed64(1),
	}

	signBuf := new(bytes.Buffer)
	crcProposalWithdraw.SerializeUnsigned(signBuf, payload.CRCProposalReviewVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalWithdraw.Sign = sig

	txn.Inputs = []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  common.EmptyHash,
				Index: math.MaxUint16,
			},
			Sequence: math.MaxUint32,
		},
	}
	txn.Outputs = []*types.Output{
		{
			AssetID:     config.ELAAssetID,
			ProgramHash: *recipient,
			Value:       recipAmout,
		},
		{
			AssetID:     config.ELAAssetID,
			ProgramHash: *commitee,
			Value:       commiteAmout,
		},
	}

	txn.Payload = crcProposalWithdraw
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(crPublicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) TestCheckCRCProposalWithdrawTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	RecipientAddress := "ERyUmNH51roR9qfru37Kqkaok2NghR7L5U"
	CRCCommitteeAddress := "8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta"
	Recipient, _ := common.Uint168FromAddress(RecipientAddress)
	tenureHeight := config.DefaultParams.CRCommitteeStartHeight
	pk1Bytes, _ := common.HexStringToBytes(publicKeyStr1)
	ela := common.Fixed64(100000000)

	references := make(map[*types.Input]*types.Output)
	references[&types.Input{}] = &types.Output{
		ProgramHash: *randomUint168(),
		Value:       common.Fixed64(60 * ela),
	}
	CRCCommitteeAddressU168, _ := common.Uint168FromAddress(CRCCommitteeAddress)
	s.Chain.chainParams.CRCAddress = *CRCCommitteeAddressU168

	// stage = 1 ok
	txn := s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 1,
		Recipient, CRCCommitteeAddressU168, 9*ela, 50*ela)
	crcProposalWithdraw, _ := txn.Payload.(*payload.CRCProposalWithdraw)
	propState := &crstate.ProposalState{
		Status: crstate.VoterAgreed,
		Proposal: payload.CRCProposal{

			SponsorPublicKey: pk1Bytes,
			Recipient:        *Recipient,
			Budgets:          []common.Fixed64{10 * ela, 20 * ela, 30 * ela},
		},
		CurrentStage:           1,
		CurrentWithdrawalStage: 0,
		ProposalLeader:         pk1Bytes,
	}
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err := s.Chain.checkTransactionOutput(tenureHeight, txn)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//CRCProposalWithdraw Stage wrong too small
	txn.Payload.(*payload.CRCProposalWithdraw).Stage = 0
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "CRCProposalWithdraw Stage wrong too small")

	//Stage != proposalState.CurrentStage
	txn.Payload.(*payload.CRCProposalWithdraw).Stage = 2
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "Stage != proposalState.CurrentStage")

	//stage =2 ok
	txn = s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 2,
		Recipient, CRCCommitteeAddressU168, 19*ela, 40*ela)
	crcProposalWithdraw, _ = txn.Payload.(*payload.CRCProposalWithdraw)
	propState.CurrentWithdrawalStage = 1
	propState.CurrentStage = 2
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//stage =3 ok
	txn = s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 3,
		Recipient, CRCCommitteeAddressU168, 29*ela, 30*ela)
	crcProposalWithdraw, _ = txn.Payload.(*payload.CRCProposalWithdraw)
	propState.CurrentWithdrawalStage = 2
	propState.CurrentStage = 3
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//len(txn.Outputs) ==0 transaction has no outputs
	rightOutPuts := txn.Outputs
	txn.Outputs = []*types.Output{}
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	s.EqualError(err, "transaction has no outputs")

	//txn.Outputs[1].ProgramHash !=CRCComitteeAddresss
	txn.Outputs = rightOutPuts
	txn.Outputs[1].ProgramHash = *Recipient
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	txn.Outputs[1].ProgramHash = *CRCCommitteeAddressU168
	s.EqualError(err, "txn.Outputs[1].ProgramHash !=CRCComitteeAddresss")

	//len(txn.Outputs) >2 CRCProposalWithdraw tx should not have over two output
	txn.Outputs = rightOutPuts
	txn.Outputs = append(txn.Outputs, &types.Output{})
	err = s.Chain.checkTransactionOutput(tenureHeight, txn)
	s.EqualError(err, "CRCProposalWithdraw tx should not have over two output")

	//transaction fee != withdrawPayload.Fee
	txn = s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 1,
		Recipient, CRCCommitteeAddressU168, 8*ela, 50*ela)
	crcProposalWithdraw, _ = txn.Payload.(*payload.CRCProposalWithdraw)
	propState = &crstate.ProposalState{
		Status: crstate.VoterAgreed,
		Proposal: payload.CRCProposal{

			SponsorPublicKey: pk1Bytes,
			Recipient:        *Recipient,
			Budgets:          []common.Fixed64{10 * ela, 20 * ela, 30 * ela},
		},
		CurrentStage:           1,
		CurrentWithdrawalStage: 0,
		ProposalLeader:         pk1Bytes,
	}
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err = s.Chain.checkTransactionFee(txn, references)
	s.EqualError(err, "transaction fee != withdrawPayload.Fee")

	//withdrawAmout == 0
	for i := 0; i < len(propState.Proposal.Budgets); i++ {
		propState.Proposal.Budgets[i] = common.Fixed64(0)
	}
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "withdrawAmout == 0")

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	pk2Bytes, _ := common.HexStringToBytes(publicKeyStr2)

	propState.ProposalLeader = pk2Bytes
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "the SponsorPublicKey is not ProposalLeader of proposal")
}

func (s *txValidatorTestSuite) TestCheckCRCProposalTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	tenureHeight := config.DefaultParams.CRCommitteeStartHeight
	nickName1 := "nickname 1"

	member1 := s.getCRMember(publicKeyStr1, privateKeyStr1, nickName1)
	memebers := make(map[common.Uint168]*crstate.CRMember)
	memebers[member1.Info.DID] = member1
	s.Chain.crCommittee.Members = memebers

	// ok
	txn := s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	err := s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.NoError(err)

	// invalid payload
	txn.Payload = &payload.CRInfo{}

	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "invalid payload")

	// invalid proposal type
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).ProposalType = 0x10
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "type of proposal should be known")

	// invalid proposal type
	txn.Payload.(*payload.CRCProposal).ProposalType = 0x00
	txn.Payload.(*payload.CRCProposal).DraftData = randomBytes(7)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "failed to check draft data hash")

	// CRSign is not signed by CR member
	txn = s.getCRCProposalTx(publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "CR sponsor should be one of the CR members")

	// invalid sponsor
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).SponsorPublicKey = []byte{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "invalid sponsor")

	// invalid sponsor signature
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	txn.Payload.(*payload.CRCProposal).SponsorPublicKey = publicKey1
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "sponsor signature check failed")

	// invalid CR sponsor signature
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).CRSign = []byte{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "CR sponsor signature check failed")

	// proposals can not more than MaxCommitteeProposalCount
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	crcProposal, _ := txn.Payload.(*payload.CRCProposal)
	proposalHashSet := crstate.NewProposalHashSet()
	for i := 0; i < int(s.Chain.chainParams.MaxCommitteeProposalCount); i++ {
		proposalHashSet.Add(*randomUint256())
	}
	s.Chain.crCommittee.GetProposalManager().ProposalHashs[crcProposal.
		CRSponsorDID] = proposalHashSet
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight)
	s.EqualError(err, "proposal is full")
}

func (s *txValidatorTestSuite) TestCheckStringField() {
	s.NoError(checkStringField("Normal", "test"))
	s.EqualError(checkStringField("", "test"), "Field test has invalid string length.")
	s.EqualError(checkStringField("I am more than 100, 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "test"), "Field test has invalid string length.")
}

func (s *txValidatorTestSuite) TestCheckTransactionDepositUTXO() {
	references := make(map[*types.Input]*types.Output)
	input := &types.Input{}
	var txn types.Transaction

	// Use the deposit UTXO in a TransferAsset transaction
	depositHash, _ := common.Uint168FromAddress("DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD")
	depositOutput := &types.Output{
		ProgramHash: *depositHash,
	}
	references[input] = depositOutput

	txn.TxType = types.TransferAsset
	err := checkTransactionDepositUTXO(&txn, references)
	s.EqualError(err, "only the ReturnDepositCoin and "+
		"ReturnCRDepositCoin transaction can use the deposit UTXO")

	// Use the deposit UTXO in a ReturnDepositCoin transaction
	txn.TxType = types.ReturnDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.NoError(err)

	// Use the standard UTXO in a ReturnDepositCoin transaction
	normalHash, _ := common.Uint168FromAddress("EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR")
	normalOutput := &types.Output{
		ProgramHash: *normalHash,
	}
	references[input] = normalOutput
	txn.TxType = types.ReturnDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.EqualError(err, "the ReturnDepositCoin and ReturnCRDepositCoin "+
		"transaction can only use the deposit UTXO")

	// Use the deposit UTXO in a ReturnDepositCoin transaction
	references[input] = depositOutput
	txn.TxType = types.ReturnCRDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.NoError(err)

	references[input] = normalOutput
	txn.TxType = types.ReturnCRDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.EqualError(err, "the ReturnDepositCoin and ReturnCRDepositCoin "+
		"transaction can only use the deposit UTXO")
}

func (s txValidatorTestSuite) TestCheckReturnDepositCoinTransaction() {
	height := uint32(1)
	_, pk, _ := crypto.GenerateKeyPair()
	depositCont, _ := contract.CreateDepositContractByPubKey(pk)
	publicKey, _ := pk.EncodePoint(true)
	// register CR
	s.Chain.state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterProducer,
				Payload: &payload.ProducerInfo{
					OwnerPublicKey: publicKey,
					NodePublicKey:  publicKey,
					NickName:       randomString(),
					Url:            randomString(),
				},
				Outputs: []*types.Output{
					{
						ProgramHash: *depositCont.ToProgramHash(),
						Value:       common.Fixed64(5000),
					},
				},
			},
		},
	}, nil)
	height++
	producer := s.Chain.state.GetProducer(publicKey)
	s.True(producer.State() == state.Pending, "register producer failed")

	for i := 0; i < 6; i++ {
		s.Chain.state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	s.True(producer.State() == state.Active, "active producer failed")

	// check a return deposit coin transaction with wrong state.
	references := make(map[*types.Input]*types.Output)
	references[&types.Input{}] = &types.Output{
		ProgramHash: *randomUint168(),
		Value:       common.Fixed64(5000 * 100000000),
	}

	code1, _ := contract.CreateStandardRedeemScript(pk)
	rdTx := &types.Transaction{
		TxType:  types.ReturnCRDepositCoin,
		Payload: &payload.ReturnDepositCoin{},
		Programs: []*program.Program{
			{Code: code1},
		},
		Outputs: []*types.Output{
			{Value: 4999 * 100000000},
		},
	}
	canceledHeight := uint32(8)
	err := s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "producer must be canceled before return deposit coin")

	// cancel CR
	s.Chain.state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.CancelProducer,
				Payload: &payload.ProcessProducer{
					OwnerPublicKey: publicKey,
				},
			},
		},
	}, nil)
	height++
	s.True(producer.State() == state.Canceled, "cancel producer failed")

	// check a return deposit coin transaction with wrong code.
	publicKey2 := "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"
	pubKeyBytes2, _ := common.HexStringToBytes(publicKey2)
	pubkey2, _ := crypto.DecodePoint(pubKeyBytes2)
	code2, _ := contract.CreateStandardRedeemScript(pubkey2)
	rdTx.Programs[0].Code = code2
	err = s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "signer must be producer")

	// check a return deposit coin transaction when not reached the
	// count of DepositLockupBlocks.
	rdTx.Programs[0].Code = code1
	err = s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2159+canceledHeight)
	s.EqualError(err, "return deposit does not meet the lockup limit")

	// check a return deposit coin transaction with wrong output amount.
	rdTx.Outputs[0].Value = 5000 * 100000000
	err = s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "overspend deposit")

	// check a correct deposit coin transaction.
	rdTx.Outputs[0].Value = 4999 * 100000000
	err = s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.NoError(err)
}

func (s txValidatorTestSuite) TestCheckReturnCRDepositCoinTransaction() {
	height := uint32(1)
	_, pk, _ := crypto.GenerateKeyPair()
	cont, _ := contract.CreateStandardContract(pk)
	code := cont.Code
	depositCont, _ := contract.CreateDepositContractByPubKey(pk)
	ct, _ := contract.CreateCRDIDContractByCode(code)
	did := ct.ToProgramHash()

	// register CR
	s.Chain.crCommittee.GetState().ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					Code:     code,
					DID:      *did,
					NickName: randomString(),
				},
				Outputs: []*types.Output{
					{
						ProgramHash: *depositCont.ToProgramHash(),
						Value:       common.Fixed64(5000),
					},
				},
			},
		},
	}, nil)
	height++
	candidate := s.Chain.crCommittee.GetState().GetCandidate(*did)
	s.True(candidate.State() == crstate.Pending, "register CR failed")

	for i := 0; i < 6; i++ {
		s.Chain.crCommittee.GetState().ProcessBlock(&types.Block{
			Header: types.Header{
				Height: height,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		height++
	}
	s.True(candidate.State() == crstate.Active, "active CR failed")

	isInVotingPeriod := func(height uint32) bool {
		return true
	}
	notInVotingPeriod := func(height uint32) bool {
		return false
	}

	references := make(map[*types.Input]*types.Output)
	references[&types.Input{}] = &types.Output{
		ProgramHash: *randomUint168(),
		Value:       common.Fixed64(5000 * 100000000),
	}

	rdTx := &types.Transaction{
		TxType:  types.ReturnCRDepositCoin,
		Payload: &payload.ReturnDepositCoin{},
		Programs: []*program.Program{
			{Code: code},
		},
		Outputs: []*types.Output{
			{Value: 4999 * 100000000},
		},
	}
	canceledHeight := uint32(8)

	// check a return cr deposit coin transaction when not unregistered in
	// voting period.
	err := s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight, isInVotingPeriod)
	s.EqualError(err, "candidate state is not canceled")

	// unregister CR
	s.Chain.crCommittee.GetState().ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.UnregisterCR,
				Payload: &payload.UnregisterCR{
					DID: *getDid(code),
				},
			},
		},
	}, nil)
	height++
	s.True(candidate.State() == crstate.Canceled, "canceled CR failed")

	publicKey2 := "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"
	pubKeyBytes2, _ := common.HexStringToBytes(publicKey2)
	pubkey2, _ := crypto.DecodePoint(pubKeyBytes2)
	code2, _ := contract.CreateStandardRedeemScript(pubkey2)

	// check a return cr deposit coin transaction with wrong code in voting period.
	rdTx.Programs[0].Code = code2
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight, isInVotingPeriod)
	s.EqualError(err, "signer must be CR candidate or member")

	// check a return cr deposit coin transaction when not reached the
	// count of DepositLockupBlocks in voting period.
	rdTx.Programs[0].Code = code
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2159+canceledHeight, isInVotingPeriod)
	s.EqualError(err, "return CR deposit does not meet the lockup limit")

	// check a return cr deposit coin transaction with wrong output amount.
	rdTx.Outputs[0].Value = 5000 * 100000000
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight, isInVotingPeriod)
	s.EqualError(err, "candidate overspend deposit")

	// check a correct return cr deposit coin transaction.
	rdTx.Outputs[0].Value = 4999 * 100000000
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight, notInVotingPeriod)
	s.NoError(err)

	// return CR deposit coin.
	s.Chain.crCommittee.GetState().ProcessBlock(&types.Block{
		Header: types.Header{
			Height: height,
		},
		Transactions: []*types.Transaction{
			rdTx,
		},
	}, nil)
	height++

	// check a return cr deposit coin transaction with the amount has returned.
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight, notInVotingPeriod)
	s.EqualError(err, "candidate is returned before")

}

func (s *txValidatorTestSuite) TestCheckOutputPayload() {
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	programHash, _ := common.Uint168FromAddress("EJMzC16Eorq9CuFCGtyMrq4Jmgw9jYCHQR")

	outputs := []*types.Output{
		{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: *programHash,
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
		{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: *programHash,
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					{
						VoteType:       outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{},
					},
				},
			},
		},
		{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: *programHash,
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
							{publicKey1, 0},
						},
					},
				},
			},
		},
		{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			Type:        types.OTVote,
			Payload: &outputpayload.VoteOutput{
				Version: 0,
				Contents: []outputpayload.VoteContent{
					{
						VoteType: outputpayload.Delegate,
						CandidateVotes: []outputpayload.CandidateVotes{
							{publicKey1, 0},
						},
					},
				},
			},
		},
	}

	err := checkOutputPayload(types.TransferAsset, outputs[0])
	s.NoError(err)

	err = checkOutputPayload(types.RechargeToSideChain, outputs[0])
	s.EqualError(err, "transaction type dose not match the output payload type")

	err = checkOutputPayload(types.TransferAsset, outputs[1])
	s.EqualError(err, "invalid public key count")

	err = checkOutputPayload(types.TransferAsset, outputs[2])
	s.EqualError(err, "duplicate candidate")

	err = checkOutputPayload(types.TransferAsset, outputs[3])
	s.EqualError(err, "output address should be standard")
}

func (s *txValidatorTestSuite) TestCheckVoteOutputs() {

	references := make(map[*types.Input]*types.Output)
	outputs := []*types.Output{{Type: types.OTNone}}
	s.NoError(s.Chain.checkVoteOutputs(0, outputs, references, nil, nil))

	publicKey1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	publicKey2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	publicKey3 := "031e12374bae471aa09ad479f66c2306f4bcc4ca5b754609a82a1839b94b4721b9"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	privateKeyStr3 := "94396a69462208b8fd96d83842855b867d3b0e663203cb31d0dfaec0362ec034"

	publicKey4 := "033279a88abf504192f36d0a8f06d66ab1fff80d2715cf3ecbd243b4db8ff2e77e"
	candidate4, _ := common.HexStringToBytes(publicKey4)

	registerCRTxn1 := s.getRegisterCRTx(publicKey1, privateKeyStr1, "nickName1")
	registerCRTxn2 := s.getRegisterCRTx(publicKey2, privateKeyStr2, "nickName2")
	registerCRTxn3 := s.getRegisterCRTx(publicKey3, privateKeyStr3, "nickName3")

	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
	}
	s.Chain.crCommittee.GetState().ProcessBlock(block, nil)
	code1 := getCodeByPubKeyStr(publicKey1)
	code2 := getCodeByPubKeyStr(publicKey2)
	code3 := getCodeByPubKeyStr(publicKey3)

	candidate1, _ := common.HexStringToBytes(publicKey1)
	candidate2, _ := common.HexStringToBytes(publicKey2)
	didCandidate1 := getDid(code1)
	didCandidate2 := getDid(code2)
	didCandidate3 := getDid(code3)

	producersMap := make(map[string]struct{})
	producersMap[publicKey1] = struct{}{}
	crsMap := make(map[common.Uint168]struct{})

	crsMap[*didCandidate1] = struct{}{}
	crsMap[*didCandidate3] = struct{}{}

	hashStr := "21c5656c65028fe21f2222e8f0cd46a1ec734cbdb6"
	hashByte, _ := common.HexStringToBytes(hashStr)
	hash, _ := common.Uint168FromBytes(hashByte)

	// Check vote output of v0 with delegate type and wrong output program hash
	outputs1 := []*types.Output{{Type: types.OTNone}}
	outputs1 = append(outputs1, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs1, references, producersMap,
		crsMap),
		"the output address of vote tx should exist in its input")

	// Check vote output of v0 with crc type and with wrong output program hash
	outputs2 := []*types.Output{{Type: types.OTNone}}
	outputs2 = append(outputs2, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate3.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs2, references,
		producersMap, crsMap),
		"the output address of vote tx should exist in its input")

	// Check vote output of v1 with wrong output program hash
	outputs3 := []*types.Output{{Type: types.OTNone}}
	outputs3 = append(outputs3, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 0},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate3.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs3, references, producersMap, crsMap),
		"the output address of vote tx should exist in its input")

	// Check vote output v0 with correct ouput program hash
	references[&types.Input{}] = &types.Output{
		ProgramHash: *hash,
	}
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs1, references, producersMap, crsMap))

	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs2, references, producersMap, crsMap))
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs3, references, producersMap, crsMap))

	// Check vote output of v0 with delegate type and invalid candidate
	outputs4 := []*types.Output{{Type: types.OTNone}}
	outputs4 = append(outputs4, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate2, 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs4, references, producersMap,
		crsMap),
		"invalid vote output payload producer candidate: "+publicKey2)

	// Check vote output of v0 with crc type and invalid candidate
	outputs5 := []*types.Output{{Type: types.OTNone}}
	outputs5 = append(outputs5, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs5, references, producersMap,
		crsMap),
		"payload VoteProducerVersion not support vote CR")

	// Check vote output of v1 with crc type and invalid candidate
	outputs6 := []*types.Output{{Type: types.OTNone}}
	outputs6 = append(outputs6, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs6, references, producersMap, crsMap),
		"invalid vote output payload CR candidate: "+didCandidate2.String())

	// Check vote output of v0 with invalid candidate
	outputs7 := []*types.Output{{Type: types.OTNone}}
	outputs7 = append(outputs7, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Payload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate2, 0},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs7, references, producersMap, crsMap),
		"invalid vote output payload producer candidate: "+publicKey2)

	// Check vote output of v1 with delegate type and wrong votes
	outputs8 := []*types.Output{{Type: types.OTNone}}
	outputs8 = append(outputs8, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 20},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs8, references, producersMap, crsMap),
		"votes larger than output amount")

	// Check vote output of v1 with crc type and wrong votes
	outputs9 := []*types.Output{{Type: types.OTNone}}
	outputs9 = append(outputs9, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate1.Bytes(), 10},
						{didCandidate3.Bytes(), 10},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs9, references, producersMap, crsMap),
		"total votes larger than output amount")

	// Check vote output of v1 with wrong votes
	outputs10 := []*types.Output{{Type: types.OTNone}}
	outputs10 = append(outputs10, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 20},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate3.Bytes(), 20},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs10, references, producersMap, crsMap),
		"votes larger than output amount")

	// Check vote output v1 with correct votes
	outputs11 := []*types.Output{{Type: types.OTNone}}
	outputs11 = append(outputs11, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 10},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate3.Bytes(), 10},
					},
				},
			},
		},
	})
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs11, references, producersMap, crsMap))

	// Check vote output of v1 with wrong votes
	outputs12 := []*types.Output{{Type: types.OTNone}}
	outputs12 = append(outputs12, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.Delegate,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate1, 1},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{didCandidate3.Bytes(), 1},
					},
				},
			},
		},
	})
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs12, references, producersMap,
		crsMap))

	// Check vote output v1 with correct votes
	proposalHashStr1 := "5df40cc0a4c6791acb5ebe89a96dd4f3fe21c94275589a65357406216a27ae36"
	proposalHash1, _ := common.Uint256FromHexString(proposalHashStr1)
	outputs13 := []*types.Output{{Type: types.OTNone}}
	outputs13 = append(outputs13, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRCProposal,
					CandidateVotes: []outputpayload.CandidateVotes{
						{proposalHash1.Bytes(), 10},
					},
				},
			},
		},
	})
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash1] =
		&crstate.ProposalState{Status: 0}
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs13, references, producersMap, crsMap))

	// Check vote output of v1 with wrong votes
	proposalHashStr2 := "9c5ab8998718e0c1c405a719542879dc7553fca05b4e89132ec8d0e88551fcc0"
	proposalHash2, _ := common.Uint256FromHexString(proposalHashStr2)
	outputs14 := []*types.Output{{Type: types.OTNone}}
	outputs14 = append(outputs14, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRCProposal,
					CandidateVotes: []outputpayload.CandidateVotes{
						{proposalHash2.Bytes(), 10},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs14, references, producersMap, crsMap),
		"invalid CRCProposal: 9c5ab8998718e0c1c405a719542879dc7553fca05b4e89132ec8d0e88551fcc0")

	// Check vote output v1 with correct votes
	outputs15 := []*types.Output{{Type: types.OTNone}}
	outputs15 = append(outputs15, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRCImpeachment,
					CandidateVotes: []outputpayload.CandidateVotes{
						{getCodeByPubKeyStr(publicKey4), 10},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs15, references, producersMap, crsMap),
		"CR sponsor should be one of the CR members")

	// Check vote output of v1 with wrong votes
	outputs16 := []*types.Output{{Type: types.OTNone}}
	outputs16 = append(outputs16, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRCImpeachment,
					CandidateVotes: []outputpayload.CandidateVotes{
						{getCodeByPubKeyStr(publicKey4), 10},
					},
				},
			},
		},
	})
	s.Chain.crCommittee.Members[common.Uint168{1, 2, 3}] = &crstate.CRMember{
		Info: payload.CRInfo{
			Code: getCodeByPubKeyStr(publicKey4),
		},
		MemberState: crstate.MemberElected,
	}
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs16, references, producersMap, crsMap))

	// Check vote output of v1 with wrong votes
	outputs17 := []*types.Output{{Type: types.OTNone}}
	outputs17 = append(outputs17, &types.Output{
		Type:        types.OTVote,
		ProgramHash: *hash,
		Value:       common.Fixed64(10),
		Payload: &outputpayload.VoteOutput{
			Version: 1,
			Contents: []outputpayload.VoteContent{
				{
					VoteType: outputpayload.CRCImpeachment,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidate4, 10},
					},
				},
			},
		},
	})
	s.Chain.crCommittee.Members[common.Uint168{1, 2, 3}] = &crstate.CRMember{
		Info: payload.CRInfo{
			Code: getCodeByPubKeyStr(publicKey4),
		},
		MemberState: crstate.MemberImpeached,
	}
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight, outputs15, references, producersMap, crsMap),
		"CR sponsor should be one of the CR members")

}

func (s *txValidatorTestSuite) TestCheckOutputProgramHash() {
	programHash := common.Uint168{}

	// empty program hash should pass
	s.NoError(checkOutputProgramHash(88813, programHash))

	// prefix standard program hash should pass
	programHash[0] = uint8(contract.PrefixStandard)
	s.NoError(checkOutputProgramHash(88813, programHash))

	// prefix multisig program hash should pass
	programHash[0] = uint8(contract.PrefixMultiSig)
	s.NoError(checkOutputProgramHash(88813, programHash))

	// prefix crosschain program hash should pass
	programHash[0] = uint8(contract.PrefixCrossChain)
	s.NoError(checkOutputProgramHash(88813, programHash))

	// other prefix program hash should not pass
	programHash[0] = 0x34
	s.Error(checkOutputProgramHash(88813, programHash))

	// other prefix program hash should pass in old version
	programHash[0] = 0x34
	s.NoError(checkOutputProgramHash(88811, programHash))
}

func TestTxValidatorSuite(t *testing.T) {
	suite.Run(t, new(txValidatorTestSuite))
}

func newCoinBaseTransaction(coinBasePayload *payload.CoinBase,
	currentHeight uint32) *types.Transaction {
	return &types.Transaction{
		Version:        0,
		TxType:         types.CoinBase,
		PayloadVersion: payload.CoinBaseVersion,
		Payload:        coinBasePayload,
		Inputs: []*types.Input{
			{
				Previous: types.OutPoint{
					TxID:  common.EmptyHash,
					Index: math.MaxUint16,
				},
				Sequence: math.MaxUint32,
			},
		},
		Attributes: []*types.Attribute{},
		LockTime:   currentHeight,
		Programs:   []*program.Program{},
	}
}
