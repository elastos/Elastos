package blockchain

import (
	"bytes"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA/blockchain/mock"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/elastos/Elastos.ELA/dpos/state"
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
	config.Parameters = config.ConfigParams{Configuration: &config.Template}
	log.NewDefault(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)

	foundation, err := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")
	if err != nil {
		s.Error(err)
	}
	FoundationAddress = *foundation
	s.foundationAddress = FoundationAddress

	heightVersions := &mock.HeightVersionsMock{}
	chainStore, err := NewChainStore("txValidatorTestSuite",
		config.MainNetParams.GenesisBlock)
	if err != nil {
		s.Error(err)
	}
	s.Chain, err = New(chainStore, &config.MainNetParams,
		heightVersions, state.NewState(&mock.ArbitratorsMock{}, &config.MainNetParams))
	if err != nil {
		s.Error(err)
	}

	s.OriginalLedger = DefaultLedger
	DefaultLedger = &Ledger{
		HeightVersions: heightVersions,
	}
}

func (s *txValidatorTestSuite) TearDownSuite() {
	s.Chain.db.Close()
	DefaultLedger = s.OriginalLedger
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
	tx.Inputs[0].Previous.Index = math.MaxUint16
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
	err := checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress},
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "Asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30% (CheckTxOut version)
	totalReward := config.MainNetParams.RewardPerBlock
	fmt.Printf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// reward to foundation in coinbase < 30% (CheckTxOut version)
	foundationReward = common.Fixed64(float64(totalReward) * 0.299999)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "Reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 1
	tx.Outputs = nil
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = checkTransactionOutput(s.HeightVersion1, tx)
	s.EqualError(err, "asset ID in output is invalid")

	// invalid program hash
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		address := common.Uint168{}
		address[0] = 0x23
		output.ProgramHash = address
	}
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
	err := checkAttributeProgram(s.HeightVersion1, tx)
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
		err := checkAttributeProgram(s.HeightVersion1, tx)
		s.EqualError(err, fmt.Sprintf("invalid attribute usage %v", attr.Usage))
	}
	tx.Attributes = nil

	// empty programs
	tx.Programs = []*program.Program{}
	err = checkAttributeProgram(s.HeightVersion1, tx)
	s.EqualError(err, "no programs found in transaction")

	// nil program code
	p := &program.Program{}
	tx.Programs = append(tx.Programs, p)
	err = checkAttributeProgram(s.HeightVersion1, tx)
	s.EqualError(err, "invalid program code nil")

	// nil program parameter
	var code = make([]byte, 21)
	rand.Read(code)
	p = &program.Program{Code: code}
	tx.Programs = []*program.Program{p}
	err = checkAttributeProgram(s.HeightVersion1, tx)
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
		&types.Input{}: {Value: outputValue1},
	}
	s.EqualError(checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + common.Fixed64(config.Parameters.PowConfiguration.MinTxFee)},
	}
	s.NoError(checkTransactionFee(tx, references))

	// multiple output

	outputValue1 = common.Fixed64(30 * s.ELA)
	outputValue2 := common.Fixed64(70 * s.ELA)
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: outputValue1},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: outputValue2},
	}

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + outputValue2},
	}
	s.EqualError(checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + outputValue2 + common.Fixed64(config.Parameters.PowConfiguration.MinTxFee)},
	}
	s.NoError(checkTransactionFee(tx, references))
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
	txn.Payload.(*payload.SideChainPow).SignedData = signature

	//4. Run CheckSideChainPowConsensus
	s.NoError(CheckSideChainPowConsensus(txn, arbitrator1), "TestCheckSideChainPowConsensus failed.")

	s.Error(CheckSideChainPowConsensus(txn, arbitrator2), "TestCheckSideChainPowConsensus failed.")
}

func (s *txValidatorTestSuite) TestCheckDestructionAddress() {
	destructionAddress := "ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr"
	txID, _ := common.Uint256FromHexString("7e8863a503e90e6464529feb1c25d98c903e01bec00ccfea2475db4e37d7328b")
	programHash, _ := common.Uint168FromAddress(destructionAddress)
	reference := map[*types.Input]*types.Output{
		&types.Input{Previous: types.OutPoint{*txID, 1234}, Sequence: 123456}: &types.Output{ProgramHash: *programHash},
	}

	err := checkDestructionAddress(reference)
	s.EqualError(err, fmt.Sprintf("cannot use utxo in the Elastos foundation destruction address"))
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

	// Give an invalid public key in payload
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = errPublicKey
	err = s.Chain.checkRegisterProducerTransaction(txn)
	s.EqualError(err, "invalid public key in payload")

	// Invalidates the signature in payload
	txn.Payload.(*payload.ProducerInfo).OwnerPublicKey = publicKey2
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
	// 1. Generate a vote output
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	outputs := []*types.Output{
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
						Candidates: [][]byte{
							publicKey1,
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
						VoteType:   outputpayload.Delegate,
						Candidates: [][]byte{},
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
						Candidates: [][]byte{
							publicKey1,
							publicKey1,
						},
					},
				},
			},
		},
	}

	// 2. Check output payload
	err := outputs[0].Payload.(*outputpayload.VoteOutput).Validate()
	s.NoError(err)

	err = outputs[1].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid public key count")

	err = outputs[2].Payload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate candidate")
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
	updatePayload := &payload.ProducerInfo{
		OwnerPublicKey: publicKey1,
		NickName:       "",
		Url:            "",
		Location:       1,
		NetAddress:     "",
	}
	txn.Payload = updatePayload

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

	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "Field NickName has invalid string length.")

	updatePayload.NickName = "nick name"
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "Field Url has invalid string length.")

	updatePayload.Url = "www.elastos.org"
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "Field NetAddress has invalid string length.")

	updatePayload.NetAddress = "127.0.0.1:20338"
	updatePayload.OwnerPublicKey = errPublicKey
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "invalid public key in payload")

	updatePayload.OwnerPublicKey = publicKey2
	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "invalid signature in payload")

	updatePayload.OwnerPublicKey = publicKey1
	updateSignBuf := new(bytes.Buffer)
	err := updatePayload.SerializeUnsigned(updateSignBuf, payload.ProducerInfoVersion)
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
	cancelPayload := &payload.CancelProducer{
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
	s.EqualError(err, "only the ReturnDepositCoin transaction can use the deposit UTXO")

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
	s.EqualError(err, "the ReturnDepositCoin transaction can only use the deposit UTXO")
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
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*types.Attribute{},
		LockTime:   currentHeight,
		Programs:   []*program.Program{},
	}
}
