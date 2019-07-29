// Copyright (c) 2017-2019 Elastos Foundation
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

	chainStore, err := NewChainStore(test.DataPath, params.GenesisBlock)
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
		chainStore.GetHeight, func(height uint32) (*types.Block, error) {
			hash, err := chainStore.GetBlockHash(height)
			if err != nil {
				return nil, err
			}
			return chainStore.GetBlock(hash)
		}, nil)
	if err != nil {
		s.Fail("initialize arbitrator failed")
	}
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

	references := map[*types.Input]*TxReference{
		&types.Input{}: {
			output: &types.Output{Value: outputValue1},
			txtype: types.CoinBase,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*TxReference{
		&types.Input{}: {
			output: &types.Output{Value: outputValue1 + s.Chain.chainParams.MinTransactionFee},
			txtype: types.CoinBase,
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

	references = map[*types.Input]*TxReference{
		&types.Input{}: {
			output: &types.Output{Value: outputValue1 + outputValue2},
			txtype: types.CoinBase,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*TxReference{
		&types.Input{}: {
			output: &types.Output{Value: outputValue1 + outputValue2 + s.Chain.chainParams.MinTransactionFee},
			txtype: types.CoinBase,
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
	reference := map[*types.Input]*TxReference{
		&types.Input{Previous: types.OutPoint{*txID, 1234}, Sequence: 123456}: {
			output: &types.Output{
				ProgramHash: *programHash,
			},
			txtype: types.TransferAsset,
		},
	}

	err := checkDestructionAddress(reference)
	s.EqualError(err, fmt.Sprintf("cannot use utxo from the destruction address"))
}

func (s *txValidatorTestSuite) TestCheckRegisterProducerTransaction() {
	// Generate a register producer transaction
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"
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
		Code:      getCode(publicKeyStr1),
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

func getCode(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := contract.CreateStandardRedeemScript(pk)
	return redeemScript
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
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"
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
		Code:      getCode(publicKeyStr1),
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
		Code:      getCode(publicKeyStr1),
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
		Code:      getCode(publicKeyStr1),
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
	nickName1 := "nickname 1"

	hash1, _ := getDepositAddress(publicKeyStr1)
	hash2, _ := getDepositAddress(publicKeyStr2)

	txn := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1)

	code1 := getCode(publicKeyStr1)
	code2 := getCode(publicKeyStr2)
	codeStr1 := common.BytesToHexString(code1)

	did1 := getDid(code1)
	did2 := getDid(code2)

	votingHeight := config.DefaultParams.CRVotingStartHeight

	// all ok
	err := s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	//invalid payload
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

	//not in vote Period lower
	txn.Payload.(*payload.CRInfo).Url = url
	err = s.Chain.checkRegisterCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	//not in vote Period lower upper c.params.CRCommitteeStartHeight
	err = s.Chain.checkRegisterCRTransaction(txn, config.DefaultParams.CRCommitteeStartHeight+1)
	s.EqualError(err, "should create tx during voting period")

	//nickname already in use
	s.Chain.crCommittee.GetState().Nicknames[nickName1] = struct{}{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "nick name nickname 1 already inuse")

	delete(s.Chain.crCommittee.GetState().Nicknames, nickName1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	err = s.Chain.checkRegisterCRTransaction(txn, 0)
	s.EqualError(err, "should create tx during voting period")

	//did aready exist
	s.Chain.crCommittee.GetState().CodeDIDMap[codeStr1] = *did1
	s.Chain.crCommittee.GetState().ActivityCandidates[*did1] = &crstate.Candidate{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "did "+
		"67ae53989e21c3212dd9bfed6daeb56874782502dd already exist")

	delete(s.Chain.crCommittee.GetState().CodeDIDMap, codeStr1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

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
	s.EqualError(err, "producer deposit amount is insufficient")

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

	code1 := getCode(publicKeyStr1)

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
		Code:      getCode(publicKeyStr1),
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
	code1 := getCode(publicKeyStr1)
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
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) getUnregisterCRTx(publicKeyStr, privateKeyStr string) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)

	code1 := getCode(publicKeyStr1)

	txn := new(types.Transaction)
	txn.TxType = types.UnregisterCR
	txn.Version = types.TxVersion09
	unregisterCRPayload := &payload.UnregisterCR{
		Code: code1,
	}
	signBuf := new(bytes.Buffer)
	err := unregisterCRPayload.SerializeUnsigned(signBuf, payload.UnregisterCRVersion)
	s.NoError(err)
	rcSig1, err := crypto.Sign(privateKey1, signBuf.Bytes())
	s.NoError(err)
	unregisterCRPayload.Signature = rcSig1
	txn.Payload = unregisterCRPayload

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}
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
	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"

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

	publicKeyStr1 := "03c77af162438d4b7140f8544ad6523b9734cca9c7a62476d54ed5d1bddc7a39c3"
	privateKeyStr1 := "7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	votingHeight := config.DefaultParams.CRVotingStartHeight
	nickName1 := "nickname 1"

	//registe an cr to unregister
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

func (s *txValidatorTestSuite) TestCheckStringField() {
	s.NoError(checkStringField("Normal", "test"))
	s.EqualError(checkStringField("", "test"), "Field test has invalid string length.")
	s.EqualError(checkStringField("I am more than 100, 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", "test"), "Field test has invalid string length.")
}

func (s *txValidatorTestSuite) TestCheckTransactionDepositUTXO() {
	references := make(map[*types.Input]*TxReference)
	input := &types.Input{}
	var txn types.Transaction

	// Use the deposit UTXO in a TransferAsset transaction
	depositHash, _ := common.Uint168FromAddress("DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD")
	depositOutput := &types.Output{
		ProgramHash: *depositHash,
	}
	references[input] = &TxReference{
		output: depositOutput,
		txtype: types.TransferAsset,
	}

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
	references[input] = &TxReference{
		output: normalOutput,
		txtype: types.TransferAsset,
	}
	txn.TxType = types.ReturnDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.EqualError(err, "the ReturnDepositCoin and ReturnCRDepositCoin "+
		"transaction can only use the deposit UTXO")

	// Use the deposit UTXO in a ReturnDepositCoin transaction
	references[input] = &TxReference{
		output: depositOutput,
		txtype: types.TransferAsset,
	}
	txn.TxType = types.ReturnCRDepositCoin
	err = checkTransactionDepositUTXO(&txn, references)
	s.NoError(err)

	references[input] = &TxReference{
		output: normalOutput,
		txtype: types.TransferAsset,
	}
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
	references := make(map[*types.Input]*TxReference)
	references[&types.Input{}] =
		&TxReference{
			output: &types.Output{
				ProgramHash: *randomUint168(),
				Value:       common.Fixed64(5000 * 100000000),
			},
			txtype: types.TransferAsset,
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
	candidate := s.Chain.crCommittee.GetState().GetCandidate(code)
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

	references := make(map[*types.Input]*TxReference)
	references[&types.Input{}] =
		&TxReference{
			output: &types.Output{
				ProgramHash: *randomUint168(),
				Value:       common.Fixed64(5000 * 100000000),
			},
			txtype: types.TransferAsset,
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
					Code: code,
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
	s.EqualError(err, "signer must be CR candidate")

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

	references := make(map[*types.Input]*TxReference)
	outputs := []*types.Output{{Type: types.OTNone}}
	s.NoError(checkVoteOutputs(outputs, references, nil, nil))

	publicKey1 := "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a"
	publicKey2 := "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"
	publicKey3 := "039d419986f5c2bf6f2a6f59f0b6e111735b66570fb22107a038bca3e1005d1920"
	candidate1, _ := common.HexStringToBytes(publicKey1)
	candidate2, _ := common.HexStringToBytes(publicKey2)
	candidate3, _ := common.HexStringToBytes(publicKey3)
	producersMap := make(map[string]struct{})
	producersMap[publicKey1] = struct{}{}
	crsMap := make(map[string]struct{})
	crsMap[publicKey3] = struct{}{}

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
	s.EqualError(checkVoteOutputs(outputs1, references, producersMap, crsMap),
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
						{candidate3, 0},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs2, references, producersMap, crsMap),
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
						{candidate3, 0},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs3, references, producersMap, crsMap),
		"the output address of vote tx should exist in its input")

	// Check vote output v0 with correct ouput program hash
	references[&types.Input{}] = &TxReference{
		output: &types.Output{
			ProgramHash: *hash,
		},
		txtype: types.TransferAsset,
	}
	s.NoError(checkVoteOutputs(outputs1, references, producersMap, crsMap))
	s.NoError(checkVoteOutputs(outputs2, references, producersMap, crsMap))
	s.NoError(checkVoteOutputs(outputs3, references, producersMap, crsMap))

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
	s.EqualError(checkVoteOutputs(outputs4, references, producersMap, crsMap),
		"invalid vote output payload candidate: "+
			"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9")

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
						{candidate2, 0},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs5, references, producersMap, crsMap),
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
						{candidate2, 0},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs6, references, producersMap, crsMap),
		"invalid vote output payload candidate: "+
			"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9")

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
						{candidate2, 0},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs7, references, producersMap, crsMap),
		"invalid vote output payload candidate: "+
			"030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9")

	crsMap[publicKey2] = struct{}{}

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
	s.EqualError(checkVoteOutputs(outputs8, references, producersMap, crsMap),
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
						{candidate2, 10},
						{candidate3, 10},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs9, references, producersMap, crsMap),
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
						{candidate3, 20},
					},
				},
			},
		},
	})
	s.EqualError(checkVoteOutputs(outputs10, references, producersMap, crsMap),
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
						{candidate3, 10},
					},
				},
			},
		},
	})
	s.NoError(checkVoteOutputs(outputs11, references, producersMap, crsMap))

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
						{candidate3, 1},
					},
				},
			},
		},
	})
	s.NoError(checkVoteOutputs(outputs12, references, producersMap, crsMap))
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
