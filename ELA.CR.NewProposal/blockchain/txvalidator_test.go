package blockchain

import (
	"bytes"
	"crypto/elliptic"
	"crypto/rand"
	"errors"
	"fmt"
	"math"
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/version/heights"

	"github.com/stretchr/testify/suite"
)

type txValidatorTestSuite struct {
	suite.Suite

	ELA               int64
	foundationAddress common.Uint168
}

func (s *txValidatorTestSuite) SetupSuite() {
	log.Init(
		config.Parameters.PrintLevel,
		config.Parameters.MaxPerLogSize,
		config.Parameters.MaxLogsSize,
	)
	foundation, err := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")
	if err != nil {
		log.Error(err)
		os.Exit(-1)
	}
	s.foundationAddress = *foundation
}

func (s *txValidatorTestSuite) TestCheckTransactionSize() {
	tx := buildTx()
	buf := new(bytes.Buffer)
	err := tx.Serialize(buf)
	if !s.NoError(err) {
		return
	}

	size := tx.GetSize()
	// normal
	config.Parameters.MaxBlockSize = size
	err = CheckTransactionSize(tx)
	s.NoError(err, "[CheckTransactionSize] passed normal size")

	// invalid
	config.Parameters.MaxBlockSize = size - 1
	err = CheckTransactionSize(tx)
	s.EqualError(err, fmt.Sprintf("Invalid transaction size: %d bytes", size))
}

func (s *txValidatorTestSuite) TestCheckTransactionInput() {
	// coinbase transaction
	tx := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	tx.Inputs[0].Previous.Index = math.MaxUint16
	err := CheckTransactionInput(tx)
	s.NoError(err)

	// invalid coinbase refer index
	tx.Inputs[0].Previous.Index = 0
	err = CheckTransactionInput(tx)
	s.EqualError(err, "invalid coinbase input")

	// invalid coinbase refer id
	tx.Inputs[0].Previous.Index = math.MaxUint16
	rand.Read(tx.Inputs[0].Previous.TxID[:])
	err = CheckTransactionInput(tx)
	s.EqualError(err, "invalid coinbase input")

	// multiple coinbase inputs
	tx.Inputs = append(tx.Inputs, &types.Input{})
	err = CheckTransactionInput(tx)
	s.EqualError(err, "coinbase must has only one input")

	// normal transaction
	tx = buildTx()
	err = CheckTransactionInput(tx)
	s.NoError(err)

	// no inputs
	tx.Inputs = nil
	err = CheckTransactionInput(tx)
	s.EqualError(err, "transaction has no inputs")

	// normal transaction with coinbase input
	tx.Inputs = append(tx.Inputs, &types.Input{Previous: *types.NewOutPoint(common.EmptyHash, math.MaxUint16)})
	err = CheckTransactionInput(tx)
	s.EqualError(err, "invalid transaction input")

	// duplicated inputs
	tx = buildTx()
	tx.Inputs = append(tx.Inputs, tx.Inputs[0])
	err = CheckTransactionInput(tx)
	s.EqualError(err, "duplicated transaction inputs")
}

func (s *txValidatorTestSuite) TestCheckTransactionOutput() {
	// coinbase
	tx := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress},
	}
	err := CheckTransactionOutput(heights.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.EqualError(err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.EqualError(err, "Asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30% (CheckTxOut version)
	totalReward := RewardAmountPerBlock
	fmt.Printf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.NoError(err)

	// reward to foundation in coinbase < 30% (CheckTxOut version)
	foundationReward = common.Fixed64(float64(totalReward) * 0.299999)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.EqualError(err, "Reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.NoError(err)

	// outputs < 1
	tx.Outputs = nil
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.EqualError(err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	s.EqualError(err, "asset ID in output is invalid")

	// invalid program hash
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
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
	err := CheckAttributeProgram(heights.HeightVersion1, tx)
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
		err := CheckAttributeProgram(heights.HeightVersion1, tx)
		s.EqualError(err, fmt.Sprintf("invalid attribute usage %v", attr.Usage))
	}
	tx.Attributes = nil

	// empty programs
	tx.Programs = []*program.Program{}
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	s.EqualError(err, "no programs found in transaction")

	// nil program code
	p := &program.Program{}
	tx.Programs = append(tx.Programs, p)
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	s.EqualError(err, "invalid program code nil")

	// nil program parameter
	var code = make([]byte, 21)
	rand.Read(code)
	p = &program.Program{Code: code}
	tx.Programs = []*program.Program{p}
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	s.EqualError(err, "invalid program parameter nil")
}

func (s *txValidatorTestSuite) TestCheckTransactionPayload() {
	// normal
	tx := new(types.Transaction)
	payload := &payload.PayloadRegisterAsset{
		Asset: payload.Asset{
			Name:      "ELA",
			Precision: 0x08,
			AssetType: payload.Token,
		},
		Amount: 3300 * 10000 * 10000000,
	}
	tx.Payload = payload
	err := CheckTransactionPayload(tx)
	s.NoError(err)

	// invalid precision
	payload.Asset.Precision = 9
	err = CheckTransactionPayload(tx)
	s.EqualError(err, "Invalide asset Precision.")

	// invalid amount
	payload.Asset.Precision = 0
	payload.Amount = 1234567
	err = CheckTransactionPayload(tx)
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
	txn.Payload = &payload.PayloadWithdrawFromSideChain{
		BlockHeight:         100,
		GenesisBlockAddress: "eb7adb1fea0dd6185b09a43bdcd4924bb22bff7151f0b1b4e08699840ab1384b",
		SideChainTransactionHashes: []common.Uint256{
			*hash1,
			*hash2,
			*hash1, // duplicate tx hash
		},
	}

	// 2. Run CheckDuplicateSidechainTx
	err := CheckDuplicateSidechainTx(txn)
	s.EqualError(err, "Duplicate sidechain tx detected in a transaction")
}

func (s *txValidatorTestSuite) TestCheckTransactionBalance() {
	// WithdrawFromSideChain will pass check in any condition
	tx := new(types.Transaction)
	tx.TxType = types.WithdrawFromSideChain

	// single output

	outputValue1 := common.Fixed64(100 * s.ELA)
	deposit := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	deposit.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress, Value: outputValue1},
	}

	references := map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1},
	}
	s.EqualError(CheckTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + common.Fixed64(config.Parameters.PowConfiguration.MinTxFee)},
	}
	s.NoError(CheckTransactionFee(tx, references))

	// multiple output

	outputValue1 = common.Fixed64(30 * s.ELA)
	outputValue2 := common.Fixed64(70 * s.ELA)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: s.foundationAddress, Value: outputValue1},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: outputValue2},
	}

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + outputValue2},
	}
	s.EqualError(CheckTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]*types.Output{
		&types.Input{}: {Value: outputValue1 + outputValue2 + common.Fixed64(config.Parameters.PowConfiguration.MinTxFee)},
	}
	s.NoError(CheckTransactionFee(tx, references))
}

func (s *txValidatorTestSuite) TestCheckSideChainPowConsensus() {
	// 1. Generate a side chain pow transaction
	txn := new(types.Transaction)
	txn.TxType = types.SideChainPow
	txn.Payload = &payload.PayloadSideChainPow{
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
	txn.Payload.Serialize(buf, payload.SideChainPowPayloadVersion)
	signature, _ := crypto.Sign(privateKey1, buf.Bytes()[0:68])
	txn.Payload.(*payload.PayloadSideChainPow).SignedData = signature

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

	err := CheckDestructionAddress(reference)
	s.EqualError(err, fmt.Sprintf("cannot use utxo in the Elastos foundation destruction address"))
}

func (s *txValidatorTestSuite) TestCheckRegisterProducerTransaction() {
	// 1. Generate a register producer transaction
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterProducer
	txn.Payload = &payload.PayloadRegisterProducer{
		PublicKey: publicKey1,
		NickName:  "nick name 1",
		Url:       "http://www.google.com",
		Location:  1,
		Address:   "127.0.0.1",
	}

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	publicKeyPlege1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000,
		OutputLock:  0,
		ProgramHash: *publicKeyPlege1,
	}}

	// 2. Check transaction
	err := CheckRegisterProducerTransaction(txn)
	s.NoError(err)

	// 3. Change public key in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = errPublicKey

	// 4. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	s.EqualError(err, "Invalid publick key.")

	// 5. Change public key in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = publicKey2

	// 6. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	s.EqualError(err, "Public key unsigned.")

	// 7. Change url in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = publicKey1
	txn.Payload.(*payload.PayloadRegisterProducer).Url = ""

	// 8. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	s.EqualError(err, "Invalid url.")
}

func getCode(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := createStandardRedeemScript(pk)
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
		},
		&types.Output{
			AssetID:     common.Uint256{},
			Value:       1.0,
			OutputLock:  0,
			ProgramHash: common.Uint168{123},
			OutputType:  types.VoteOutput,
			OutputPayload: &outputpayload.VoteOutput{
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
			OutputType:  types.VoteOutput,
			OutputPayload: &outputpayload.VoteOutput{
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
	err := outputs[0].OutputPayload.(*outputpayload.VoteOutput).Validate()
	s.NoError(err)

	err = outputs[1].OutputPayload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "invalid public key count")

	err = outputs[2].OutputPayload.(*outputpayload.VoteOutput).Validate()
	s.EqualError(err, "duplicate candidate")
}

func (s *txValidatorTestSuite) TestCheckUpdateProducerTransaction() {
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterProducer
	updatePayload := &payload.PayloadUpdateProducer{
		PublicKey: publicKey1,
		NickName:  "",
		Url:       "",
		Location:  1,
		Address:   "",
	}
	txn.Payload = updatePayload

	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	s.EqualError(CheckUpdateProducerTransaction(txn), "Invalid nick name.")

	updatePayload.NickName = "nick name"
	s.EqualError(CheckUpdateProducerTransaction(txn), "Invalid url.")

	updatePayload.Url = "www.elastos.org"
	s.EqualError(CheckUpdateProducerTransaction(txn), "Invalid IP.")

	updatePayload.Address = "127.0.0.1:20338"
	updatePayload.PublicKey = errPublicKey
	s.EqualError(CheckUpdateProducerTransaction(txn), "Invalid publick key.")

	updatePayload.PublicKey = publicKey2
	s.EqualError(CheckUpdateProducerTransaction(txn), "Public key unsigned.")

	updatePayload.PublicKey = publicKey1
	s.EqualError(CheckUpdateProducerTransaction(txn), "Invalid producer.")

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
	cancelPayload := &payload.PayloadCancelProducer{
		PublicKey: publicKey1,
	}
	txn.Payload = cancelPayload

	txn.Programs = []*program.Program{{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	cancelPayload.PublicKey = errPublicKey
	s.EqualError(CheckCancelProducerTransaction(txn), "Invalid publick key.")

	cancelPayload.PublicKey = publicKey2
	s.EqualError(CheckCancelProducerTransaction(txn), "Public key unsigned.")
}

func TestTxValidatorSuite(t *testing.T) {
	suite.Run(t, new(txValidatorTestSuite))
}

func createStandardRedeemScript(publicKey *crypto.PublicKey) ([]byte, error) {
	content, err := publicKey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("create standard redeem script, encode public key failed")
	}
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(content)))
	buf.Write(content)
	buf.WriteByte(byte(common.STANDARD))

	return buf.Bytes(), nil
}
