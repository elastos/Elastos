// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"crypto/elliptic"
	"crypto/rand"
	crand "crypto/rand"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"math"
	mrand "math/rand"
	"path/filepath"
	"testing"

	elaact "github.com/elastos/Elastos.ELA/account"
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
	CurrentHeight     uint32
	Chain             *BlockChain
	OriginalLedger    *Ledger
}

func init() {
	testing.Init()
}

func (s *txValidatorTestSuite) SetupSuite() {
	log.NewDefault(test.NodeLogPath, 0, 0, 0)

	params := &config.DefaultParams
	FoundationAddress = params.Foundation
	s.foundationAddress = params.Foundation

	chainStore, err := NewChainStore(filepath.Join(test.DataPath, "txvalidator"), params)
	if err != nil {
		s.Error(err)
	}
	s.Chain, err = New(chainStore, params,
		state.NewState(params, nil, nil),
		crstate.NewCommittee(params))
	if err != nil {
		s.Error(err)
	}
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          chainStore.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})

	if err := s.Chain.Init(nil); err != nil {
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
			return nil, nil
		}, nil)
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
	blockHeight3 := s.Chain.chainParams.RegisterCRByDIDHeight

	// check height version of registerCR transaction.
	registerCR := &types.Transaction{TxType: types.RegisterCR}
	err := s.Chain.checkTxHeightVersion(registerCR, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(registerCR, blockHeight2)
	s.NoError(err)

	registerCR2 := &types.Transaction{TxType: types.RegisterCR,
		PayloadVersion: payload.CRInfoDIDVersion}
	err = s.Chain.checkTxHeightVersion(registerCR2, blockHeight1)
	s.EqualError(err, "not support before CRVotingStartHeight")
	err = s.Chain.checkTxHeightVersion(registerCR2, blockHeight3)
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
	err := s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.NoError(err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: s.foundationAddress},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30% (CheckTxOut version)
	totalReward := config.DefaultParams.RewardPerBlock
	fmt.Printf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.NoError(err)

	// reward to foundation in coinbase < 30% (CheckTxOut version)
	foundationReward = common.Fixed64(float64(totalReward) * 0.299999)
	fmt.Printf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: config.ELAAssetID, ProgramHash: s.foundationAddress, Value: foundationReward},
		{AssetID: config.ELAAssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = config.ELAAssetID
		output.ProgramHash = common.Uint168{}
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.NoError(err)

	// outputs < 1
	tx.Outputs = nil
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
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
	s.NoError(s.Chain.checkTransactionOutput(tx, s.HeightVersion1))
	tx.Outputs = appendSpecial() // add another special output here
	originHeight := config.DefaultParams.PublicDPOSHeight
	config.DefaultParams.PublicDPOSHeight = 0
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
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
	s.NoError(s.Chain.checkTransactionOutput(tx, s.HeightVersion1))
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
	s.NoError(s.Chain.checkTransactionOutput(tx, s.HeightVersion1))

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
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "new sideChainPow tx must have only one output")

	tx.Outputs = []*types.Output{
		{
			Value: 100,
			Type:  0,
		},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
	s.EqualError(err, "the value of new sideChainPow tx output must be 0")

	tx.Outputs = []*types.Output{
		{
			Value: 0,
			Type:  1,
		},
	}
	err = s.Chain.checkTransactionOutput(tx, s.HeightVersion1)
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

	references := map[*types.Input]types.Output{
		&types.Input{}: {
			Value: outputValue1,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]types.Output{
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

	references = map[*types.Input]types.Output{
		&types.Input{}: {
			Value: outputValue1 + outputValue2,
		},
	}
	s.EqualError(s.Chain.checkTransactionFee(tx, references), "transaction fee not enough")

	references = map[*types.Input]types.Output{
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
	reference := map[*types.Input]types.Output{
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
	s.NoError(err)

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
	s.NoError(err)

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

	s.CurrentHeight = 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.state = state.NewState(s.Chain.chainParams, nil, func(programHash common.Uint168) (common.Fixed64,
		error) {
		amount := common.Fixed64(0)
		utxos, err := s.Chain.db.GetFFLDB().GetUTXO(&programHash)
		if err != nil {
			return amount, err
		}
		for _, utxo := range utxos {
			amount += utxo.Value
		}
		return amount, nil
	})
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	block := &types.Block{
		Transactions: []*types.Transaction{
			txn,
		},
		Header: types.Header{Height: s.CurrentHeight},
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
	s.CurrentHeight++
	block.Header = types.Header{Height: s.CurrentHeight}
	s.Chain.state.ProcessBlock(block, nil)

	s.EqualError(s.Chain.checkUpdateProducerTransaction(txn), "field NickName has invalid string length")
	updatePayload.NickName = "nick name"

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

	txn := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1,
		payload.CRInfoVersion, &common.Uint168{})

	code1 := getCodeByPubKeyStr(publicKeyStr1)
	code2 := getCodeByPubKeyStr(publicKeyStr2)
	codeStr1 := common.BytesToHexString(code1)

	cid1 := getCID(code1)
	cid2 := getCID(code2)

	votingHeight := config.DefaultParams.CRVotingStartHeight
	registerCRByDIDHeight := config.DefaultParams.RegisterCRByDIDHeight

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
	s.EqualError(err, "field NickName has invalid string length")

	// Give an invalid NickName length more than 100 in payload
	txn.Payload.(*payload.CRInfo).NickName = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "field NickName has invalid string length")

	// Give an invalid url length more than 100 in payload
	url := txn.Payload.(*payload.CRInfo).Url
	txn.Payload.(*payload.CRInfo).NickName = nickName
	txn.Payload.(*payload.CRInfo).Url = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "field Url has invalid string length")

	// Not in vote Period lower
	txn.Payload.(*payload.CRInfo).Url = url
	err = s.Chain.checkRegisterCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	// Not in vote Period upper c.params.CRCommitteeStartHeight
	s.Chain.crCommittee.InElectionPeriod = true
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

	delete(s.Chain.crCommittee.GetState().CodeCIDMap, codeStr1)
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	// CID already exist
	s.Chain.crCommittee.GetState().CodeCIDMap[codeStr1] = *cid1
	s.Chain.crCommittee.GetState().Candidates[*cid1] = &crstate.Candidate{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "cid "+cid1.String()+" already exist")
	delete(s.Chain.crCommittee.GetState().Candidates, *cid1)

	// Give an invalid code in payload
	txn.Payload.(*payload.CRInfo).Code = []byte{}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "code is nil")

	// Give an invalid CID in payload
	txn.Payload.(*payload.CRInfo).Code = code1
	txn.Payload.(*payload.CRInfo).CID = common.Uint168{1, 2, 3}
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "invalid cid address")

	// Give a mismatching code and CID in payload
	txn.Payload.(*payload.CRInfo).CID = *cid2
	err = s.Chain.checkRegisterCRTransaction(txn, votingHeight)
	s.EqualError(err, "invalid cid address")

	// Invalidates the signature in payload
	txn.Payload.(*payload.CRInfo).CID = *cid1
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

	//check register cr with CRInfoDIDVersion
	txn2 := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1,
		payload.CRInfoDIDVersion, &common.Uint168{1, 2, 3})
	err = s.Chain.checkRegisterCRTransaction(txn2, registerCRByDIDHeight)
	s.EqualError(err, "invalid did address")

	did2, _ := getDIDFromCode(code2)
	txn2 = s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1,
		payload.CRInfoDIDVersion, did2)
	err = s.Chain.checkRegisterCRTransaction(txn2, registerCRByDIDHeight)
	s.EqualError(err, "invalid did address")

	did1, _ := getDIDFromCode(code1)
	txn2 = s.getRegisterCRTx(publicKeyStr1, privateKeyStr1, nickName1,
		payload.CRInfoDIDVersion, did1)
	err = s.Chain.checkRegisterCRTransaction(txn2, registerCRByDIDHeight)
	s.NoError(err)
}

func getDepositAddress(publicKeyStr string) (*common.Uint168, error) {
	publicKey, _ := common.HexStringToBytes(publicKeyStr)
	hash, err := contract.PublicKeyToDepositProgramHash(publicKey)
	if err != nil {
		return nil, err
	}
	return hash, nil
}

func getCID(code []byte) *common.Uint168 {
	ct1, _ := contract.CreateCRIDContractByCode(code)
	return ct1.ToProgramHash()
}

func (s *txValidatorTestSuite) getRegisterCRTx(publicKeyStr, privateKeyStr,
	nickName string, payloadVersion byte, did *common.Uint168) *types.Transaction {

	publicKeyStr1 := publicKeyStr
	privateKeyStr1 := privateKeyStr
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	privateKey1, _ := common.HexStringToBytes(privateKeyStr1)

	code1 := getCodeByPubKeyStr(publicKeyStr1)
	ct1, _ := contract.CreateCRIDContractByCode(code1)
	cid1 := ct1.ToProgramHash()

	hash1, _ := contract.PublicKeyToDepositProgramHash(publicKey1)

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	txn.PayloadVersion = payloadVersion
	crInfoPayload := &payload.CRInfo{
		Code:     code1,
		CID:      *cid1,
		DID:      *did,
		NickName: nickName,
		Url:      "http://www.elastos_test.com",
		Location: 1,
	}
	signBuf := new(bytes.Buffer)
	crInfoPayload.SerializeUnsigned(signBuf, payloadVersion)
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

	ctDID, _ := contract.CreateCRIDContractByCode(multiCode)
	cid := ctDID.ToProgramHash()

	ctDeposit, _ := contract.CreateDepositContractByCode(multiCode)
	deposit := ctDeposit.ToProgramHash()

	txn := new(types.Transaction)
	txn.TxType = types.RegisterCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     multiCode,
		CID:      *cid,
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
	ct1, _ := contract.CreateCRIDContractByCode(code1)
	cid1 := ct1.ToProgramHash()

	txn := new(types.Transaction)
	txn.TxType = types.UpdateCR
	txn.Version = types.TxVersion09
	crInfoPayload := &payload.CRInfo{
		Code:     code1,
		CID:      *cid1,
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
		CID: *getCID(code1),
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
	did1, _ := getDIDFromCode(code1)

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

	recipient := *randomUint168()
	recipient[0] = uint8(contract.PrefixStandard)
	did2, _ := getDIDFromCode(code2)
	crcProposalPayload := &payload.CRCProposal{
		ProposalType:       payload.Normal,
		OwnerPublicKey:     publicKey1,
		CRCouncilMemberDID: *did2,
		DraftHash:          common.Hash(draftData),
		Budgets:            createBudgets(3),
		Recipient:          recipient,
	}

	signBuf := new(bytes.Buffer)
	crcProposalPayload.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalPayload.Signature = sig

	common.WriteVarBytes(signBuf, sig)
	crcProposalPayload.CRCouncilMemberDID.Serialize(signBuf)
	crSig, _ := crypto.Sign(privateKey2, signBuf.Bytes())
	crcProposalPayload.CRCouncilMemberSignature = crSig

	txn.Payload = crcProposalPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func (s *txValidatorTestSuite) createSpecificStatusProposal(publicKey1, publicKey2 []byte, height uint32, status crstate.ProposalStatus) (*crstate.ProposalState, *payload.CRCProposal) {
	draftData := randomBytes(10)
	recipient := *randomUint168()
	recipient[0] = uint8(contract.PrefixStandard)
	code2 := getCodeByPubKeyStr(hex.EncodeToString(publicKey2))
	crCouncilMemberDID, _ := getDIDFromCode(code2)
	proposal := &payload.CRCProposal{
		ProposalType:       payload.Normal,
		OwnerPublicKey:     publicKey1,
		CRCouncilMemberDID: *crCouncilMemberDID,
		DraftHash:          common.Hash(draftData),
		Budgets:            createBudgets(3),
		Recipient:          recipient,
	}
	budgetsStatus := make(map[uint8]crstate.BudgetStatus)
	for _, budget := range proposal.Budgets {
		if budget.Type == payload.Imprest {
			budgetsStatus[budget.Stage] = crstate.Withdrawable
			continue
		}
		budgetsStatus[budget.Stage] = crstate.Unfinished
	}
	proposalState := &crstate.ProposalState{
		Status:              status,
		Proposal:            *proposal,
		TxHash:              common.Hash(randomBytes(10)),
		CRVotes:             map[common.Uint168]payload.VoteResult{},
		VotersRejectAmount:  common.Fixed64(0),
		RegisterHeight:      height,
		VoteStartHeight:     0,
		WithdrawnBudgets:    make(map[uint8]common.Fixed64),
		WithdrawableBudgets: make(map[uint8]common.Fixed64),
		BudgetsStatus:       budgetsStatus,
		FinalPaymentStatus:  false,
		TrackingCount:       0,
		TerminatedHeight:    0,
		ProposalOwner:       proposal.OwnerPublicKey,
	}
	return proposalState, proposal
}

func (s *txValidatorTestSuite) getCRCCloseProposalTx(publicKeyStr, privateKeyStr,
	crPublicKeyStr, crPrivateKeyStr string) *types.Transaction {

	privateKey1, _ := common.HexStringToBytes(privateKeyStr)

	privateKey2, _ := common.HexStringToBytes(crPrivateKeyStr)
	publicKey2, _ := common.HexStringToBytes(crPublicKeyStr)
	code2 := getCodeByPubKeyStr(crPublicKeyStr)
	did2, _ := getDIDFromCode(code2)

	draftData := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposal
	txn.Version = types.TxVersion09

	crcProposalPayload := &payload.CRCProposal{
		ProposalType:       payload.CloseProposal,
		OwnerPublicKey:     publicKey2,
		CRCouncilMemberDID: *did2,
		DraftHash:          common.Hash(draftData),
		CloseProposalHash:  common.Hash(randomBytes(10)),
	}

	signBuf := new(bytes.Buffer)
	crcProposalPayload.SerializeUnsigned(signBuf, payload.CRCProposalVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalPayload.Signature = sig

	common.WriteVarBytes(signBuf, sig)
	crcProposalPayload.CRCouncilMemberDID.Serialize(signBuf)
	crSig, _ := crypto.Sign(privateKey2, signBuf.Bytes())
	crcProposalPayload.CRCouncilMemberSignature = crSig

	txn.Payload = crcProposalPayload
	txn.Programs = []*program.Program{&program.Program{
		Code:      getCodeByPubKeyStr(publicKeyStr),
		Parameter: nil,
	}}
	return txn
}

func createBudgets(n int) []payload.Budget {
	budgets := make([]payload.Budget, 0)
	for i := 0; i < n; i++ {
		var budgetType = payload.NormalPayment
		if i == 0 {
			budgetType = payload.Imprest
		}
		if i == n-1 {
			budgetType = payload.FinalPayment
		}
		budget := &payload.Budget{
			Stage:  byte(i),
			Type:   budgetType,
			Amount: common.Fixed64((i + 1) * 1e8),
		}
		budgets = append(budgets, *budget)
	}
	return budgets
}

func randomFix64() common.Fixed64 {
	var randNum int64
	binary.Read(crand.Reader, binary.BigEndian, &randNum)
	return common.Fixed64(randNum)
}

func (s *txValidatorTestSuite) TestCheckCRCProposalTrackingTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	publicKeyStr3 := "024010e8ac9b2175837dac34917bdaf3eb0522cff8c40fc58419d119589cae1433"
	privateKeyStr3 := "e19737ffeb452fc7ed9dc0e70928591c88ad669fd1701210dcd8732e0946829b"

	ownerPubKey, _ := common.HexStringToBytes(publicKeyStr1)

	proposalHash := randomUint256()
	recipient := randomUint168()
	votingHeight := config.DefaultParams.CRVotingStartHeight

	// Set secretary general.
	s.Chain.chainParams.SecretaryGeneral = publicKeyStr3

	// Check Common tracking tx.
	txn := s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:       0,
				OwnerPublicKey:     ownerPubKey,
				CRCouncilMemberDID: *randomUint168(),
				DraftHash:          *randomUint256(),
				Budgets:            createBudgets(3),
				Recipient:          *recipient,
			},
			Status:        crstate.VoterAgreed,
			ProposalOwner: ownerPubKey,
		}

	err := s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage should assignment zero value")

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewOwnerPublicKey need to be empty")

	// Check Progress tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 1,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Progress, *proposalHash, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewOwnerPublicKey need to be empty")

	// Check Terminated tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage should assignment zero value")

	txn = s.getCRCProposalTrackingTx(payload.Terminated, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "the NewOwnerPublicKey need to be empty")

	// Check ChangeOwner tracking tx.
	txn = s.getCRCProposalTrackingTx(payload.ChangeOwner, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.NoError(err)

	txn = s.getCRCProposalTrackingTx(payload.ChangeOwner, *proposalHash, 1,
		publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2,
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "stage should assignment zero value")

	txn = s.getCRCProposalTrackingTx(payload.ChangeOwner, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "invalid new proposal owner public key")

	// Check invalid proposal hash.
	txn = s.getCRCProposalTrackingTx(payload.Common, *randomUint256(), 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "proposal not exist")

	txn = s.getCRCProposalTrackingTx(payload.Common, *proposalHash, 0,
		publicKeyStr1, privateKeyStr1, "", "",
		publicKeyStr3, privateKeyStr3)

	// Check proposal status is not VoterAgreed.
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:       0,
				OwnerPublicKey:     ownerPubKey,
				CRCouncilMemberDID: *randomUint168(),
				DraftHash:          *randomUint256(),
				Budgets:            createBudgets(3),
				Recipient:          *recipient,
			},
			TerminatedHeight: 100,
			Status:           crstate.VoterCanceled,
			ProposalOwner:    ownerPubKey,
		}
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash].TerminatedHeight = 100
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "proposal status is not VoterAgreed")

	// Check reach max proposal tracking count.
	s.Chain.crCommittee.GetProposalManager().Proposals[*proposalHash] =
		&crstate.ProposalState{
			Proposal: payload.CRCProposal{
				ProposalType:       0,
				OwnerPublicKey:     ownerPubKey,
				CRCouncilMemberDID: *randomUint168(),
				DraftHash:          *randomUint256(),
				Budgets:            createBudgets(3),
				Recipient:          *recipient,
			},
			TrackingCount: 128,
			Status:        crstate.VoterAgreed,
			ProposalOwner: ownerPubKey,
		}
	err = s.Chain.checkCRCProposalTrackingTransaction(txn, votingHeight)
	s.EqualError(err, "reached max tracking count")

}

func (s *txValidatorTestSuite) getCRCProposalTrackingTx(
	trackingType payload.CRCProposalTrackingType,
	proposalHash common.Uint256, stage uint8,
	ownerPublicKeyStr, ownerPrivateKeyStr,
	newLeaderPublicKeyStr, newLeaderPrivateKeyStr,
	sgPublicKeyStr, sgPrivateKeyStr string) *types.Transaction {

	ownerPublicKey, _ := common.HexStringToBytes(ownerPublicKeyStr)
	ownerPrivateKey, _ := common.HexStringToBytes(ownerPrivateKeyStr)

	newLeaderPublicKey, _ := common.HexStringToBytes(newLeaderPublicKeyStr)
	newLeaderPrivateKey, _ := common.HexStringToBytes(newLeaderPrivateKeyStr)

	sgPrivateKey, _ := common.HexStringToBytes(sgPrivateKeyStr)

	documentData := randomBytes(10)
	opinionHash := randomBytes(10)
	txn := new(types.Transaction)
	txn.TxType = types.CRCProposalTracking
	txn.Version = types.TxVersion09
	cPayload := &payload.CRCProposalTracking{
		ProposalTrackingType:        trackingType,
		ProposalHash:                proposalHash,
		Stage:                       stage,
		MessageHash:                 common.Hash(documentData),
		OwnerPublicKey:              ownerPublicKey,
		NewOwnerPublicKey:           newLeaderPublicKey,
		SecretaryGeneralOpinionHash: common.Hash(opinionHash),
	}

	signBuf := new(bytes.Buffer)
	cPayload.SerializeUnsigned(signBuf, payload.CRCProposalTrackingVersion)
	sig, _ := crypto.Sign(ownerPrivateKey, signBuf.Bytes())
	cPayload.OwnerSignature = sig

	if newLeaderPublicKeyStr != "" && newLeaderPrivateKeyStr != "" {
		common.WriteVarBytes(signBuf, sig)
		crSig, _ := crypto.Sign(newLeaderPrivateKey, signBuf.Bytes())
		cPayload.NewOwnerSignature = crSig
		sig = crSig
	}

	common.WriteVarBytes(signBuf, sig)
	cPayload.SecretaryGeneralOpinionHash.Serialize(signBuf)
	crSig, _ := crypto.Sign(sgPrivateKey, signBuf.Bytes())
	cPayload.SecretaryGeneralSignature = crSig

	txn.Payload = cPayload
	return txn
}

func (s *txValidatorTestSuite) TestCheckCRCAppropriationTransaction() {
	// Set CRC foundation and CRC committee address.
	s.Chain.chainParams.CRCFoundation = *randomUint168()
	s.Chain.chainParams.CRCCommitteeAddress = *randomUint168()

	// Set CRC foundation and CRC committee amount.
	s.Chain.crCommittee.CRCFoundationBalance = common.Fixed64(900 * 1e8)
	s.Chain.crCommittee.AppropriationAmount = common.Fixed64(90 * 1e8)
	s.Chain.crCommittee.CRCCommitteeUsedAmount = common.Fixed64(0 * 1e8)

	// Create reference.
	reference := make(map[*types.Input]types.Output)
	input := &types.Input{
		Previous: types.OutPoint{
			TxID:  *randomUint256(),
			Index: 0,
		},
	}
	refOutput := types.Output{
		Value:       900 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCFoundation,
	}
	refOutputErr := types.Output{
		Value:       900 * 1e8,
		ProgramHash: *randomUint168(),
	}
	reference[input] = refOutput

	// Create CRC appropriation transaction.
	output1 := &types.Output{
		Value:       90 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCCommitteeAddress,
	}
	output2 := &types.Output{
		Value:       810 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCFoundation,
	}
	output1Err := &types.Output{
		Value:       91 * 1e8,
		ProgramHash: s.Chain.chainParams.CRCCommitteeAddress,
	}
	output2Err := &types.Output{
		Value:       809 * 1e8,
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
		"amount, inputs:900 outputs:899")

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
		CID:      *hash1,
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
	err = s.Chain.crInfoSanityCheck(rcPayload, payload.CRInfoVersion)
	s.NoError(err)

	//invalid code
	rcPayload.Code = []byte{1, 2, 3, 4, 5}
	err = s.Chain.crInfoSanityCheck(rcPayload, payload.CRInfoVersion)
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
	registerCRTxn1 := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1,
		nickName1, payload.CRInfoVersion, &common.Uint168{})
	registerCRTxn2 := s.getRegisterCRTx(publicKeyStr2, privateKeyStr2,
		nickName2, payload.CRInfoDIDVersion, &common.Uint168{})

	s.CurrentHeight = s.Chain.chainParams.CRVotingStartHeight + 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
		},
		Header: types.Header{Height: s.CurrentHeight},
	}
	s.Chain.crCommittee.ProcessBlock(block, nil)

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
	s.EqualError(err, "field NickName has invalid string length")

	// Give an invalid NickName length more than 100 in payload
	txn.Payload.(*payload.CRInfo).NickName = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).NickName = nickName
	s.EqualError(err, "field NickName has invalid string length")

	// Give an invalid url length more than 100 in payload
	url := txn.Payload.(*payload.CRInfo).Url
	txn.Payload.(*payload.CRInfo).Url = "012345678901234567890123456789012345678901234567890" +
		"12345678901234567890123456789012345678901234567890123456789"
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Url = url
	s.EqualError(err, "field Url has invalid string length")

	// Give an invalid code in payload
	code := txn.Payload.(*payload.CRInfo).Code
	txn.Payload.(*payload.CRInfo).Code = []byte{1, 2, 3, 4, 5}
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Code = code
	s.EqualError(err, "invalid cid address")

	// Give an invalid CID in payload
	cid := txn.Payload.(*payload.CRInfo).CID
	txn.Payload.(*payload.CRInfo).CID = common.Uint168{1, 2, 3}
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).CID = cid
	s.EqualError(err, "invalid cid address")

	// Give a mismatching code and CID in payload
	txn.Payload.(*payload.CRInfo).CID = *hash2
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).CID = cid
	s.EqualError(err, "invalid cid address")

	// Invalidates the signature in payload
	signatur := txn.Payload.(*payload.CRInfo).Signature
	txn.Payload.(*payload.CRInfo).Signature = randomSignature()
	err = s.Chain.checkUpdateCRTransaction(txn, votingHeight)
	txn.Payload.(*payload.CRInfo).Signature = signatur
	s.EqualError(err, "[Validation], Verify failed.")

	//not in vote Period lower
	err = s.Chain.checkUpdateCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	// set RegisterCRByDIDHeight after CRCommitteeStartHeight
	s.Chain.chainParams.RegisterCRByDIDHeight = config.DefaultParams.CRCommitteeStartHeight + 10

	//not in vote Period lower upper c.params.CRCommitteeStartHeight
	s.Chain.crCommittee.InElectionPeriod = true
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
	registerCRTxn := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1,
		nickName1, payload.CRInfoVersion, &common.Uint168{})
	s.CurrentHeight = 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn,
		},
		Header: types.Header{Height: s.CurrentHeight},
	}
	s.Chain.crCommittee.ProcessBlock(block, nil)
	//ok
	txn := s.getUnregisterCRTx(publicKeyStr1, privateKeyStr1)
	err := s.Chain.checkUnRegisterCRTransaction(txn, votingHeight)
	s.NoError(err)

	//invalid payload need unregisterCR pass registerCr
	registerTx := s.getRegisterCRTx(publicKeyStr1, privateKeyStr1,
		nickName1, payload.CRInfoVersion, &common.Uint168{})
	err = s.Chain.checkUnRegisterCRTransaction(registerTx, votingHeight)
	s.EqualError(err, "invalid payload")

	//not in vote Period lower
	err = s.Chain.checkUnRegisterCRTransaction(txn, config.DefaultParams.CRVotingStartHeight-1)
	s.EqualError(err, "should create tx during voting period")

	//not in vote Period lower upper c.params.CRCommitteeStartHeight
	s.Chain.crCommittee.InElectionPeriod = true
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
	did, _ := getDIDFromCode(code)
	crcProposalReviewPayload := &payload.CRCProposalReview{
		ProposalHash: *randomUint256(),
		VoteResult:   payload.Approve,
		DID:          *did,
	}

	signBuf := new(bytes.Buffer)
	crcProposalReviewPayload.SerializeUnsigned(signBuf, payload.CRCProposalReviewVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalReviewPayload.Signature = sig

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
	manager := s.Chain.crCommittee.GetProposalManager()
	manager.Proposals[crcProposalReview.ProposalHash] = &crstate.ProposalState{
		Status: crstate.Registered,
	}
	err := s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.NoError(err)

	// member status is not elected
	member1.MemberState = crstate.MemberImpeached
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "should be an elected CR members")

	// invalid payload
	txn.Payload = &payload.CRInfo{}
	member1.MemberState = crstate.MemberElected
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "invalid payload")

	// invalid content type
	txn = s.getCRCProposalReviewTx(publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposalReview).VoteResult = 0x10
	crcProposalReview2, _ := txn.Payload.(*payload.CRCProposalReview)
	manager.Proposals[crcProposalReview2.ProposalHash] = &crstate.ProposalState{
		Status: crstate.Registered,
	}
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "VoteResult should be known")

	// proposal reviewer is not CR member
	txn = s.getCRCProposalReviewTx(publicKeyStr2, privateKeyStr2)
	crcProposalReview3, _ := txn.Payload.(*payload.CRCProposalReview)
	manager.Proposals[crcProposalReview3.ProposalHash] = &crstate.ProposalState{
		Status: crstate.Registered,
	}
	err = s.Chain.checkCRCProposalReviewTransaction(txn, tenureHeight)
	s.EqualError(err, "did correspond crMember not exists")

	delete(manager.Proposals, crcProposalReview.ProposalHash)
	// invalid CR proposal reviewer signature
	txn = s.getCRCProposalReviewTx(publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposalReview).Signature = []byte{}
	crcProposalReview, _ = txn.Payload.(*payload.CRCProposalReview)
	manager.Proposals[crcProposalReview.ProposalHash] = &crstate.ProposalState{
		Status: crstate.Registered,
	}
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
		ProposalHash:   *randomUint256(),
		OwnerPublicKey: pkBytes,
	}

	signBuf := new(bytes.Buffer)
	crcProposalWithdraw.SerializeUnsigned(signBuf, payload.CRCProposalReviewVersion)
	sig, _ := crypto.Sign(privateKey1, signBuf.Bytes())
	crcProposalWithdraw.Signature = sig

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
	NOCRCCommitteeAddress := "EWm2ZGeSyDBBAsVSsvSvspPKV4wQBKPjUk"
	Recipient, _ := common.Uint168FromAddress(RecipientAddress)
	tenureHeight := config.DefaultParams.CRCommitteeStartHeight
	pk1Bytes, _ := common.HexStringToBytes(publicKeyStr1)
	ela := common.Fixed64(100000000)
	CRCCommitteeAddressU168, _ := common.Uint168FromAddress(CRCCommitteeAddress)
	NOCRCCommitteeAddressU168, _ := common.Uint168FromAddress(NOCRCCommitteeAddress)

	inputs := []*types.Input{
		{
			Previous: types.OutPoint{
				TxID:  common.EmptyHash,
				Index: 1,
			},
			Sequence: math.MaxUint32,
		},
	}
	outputs := []*types.Output{
		{
			AssetID:     config.ELAAssetID,
			ProgramHash: *CRCCommitteeAddressU168,
			Value:       common.Fixed64(60 * ela),
		},
		{
			AssetID:     config.ELAAssetID,
			ProgramHash: *NOCRCCommitteeAddressU168,
			Value:       common.Fixed64(600 * ela),
		},
	}

	references := make(map[*types.Input]types.Output)
	references[inputs[0]] = *outputs[0]

	s.Chain.chainParams.CRCCommitteeAddress = *CRCCommitteeAddressU168
	// stage = 1 ok
	txn := s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 1,
		Recipient, CRCCommitteeAddressU168, 9*ela, 50*ela)
	crcProposalWithdraw, _ := txn.Payload.(*payload.CRCProposalWithdraw)
	propState := &crstate.ProposalState{
		Status: crstate.VoterAgreed,
		Proposal: payload.CRCProposal{
			OwnerPublicKey: pk1Bytes,
			Recipient:      *Recipient,
			Budgets:        createBudgets(3),
		},
		FinalPaymentStatus:  false,
		WithdrawableBudgets: map[uint8]common.Fixed64{0: 10 * 1e8},
		ProposalOwner:       pk1Bytes,
	}
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err := s.Chain.checkTransactionOutput(txn, tenureHeight)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//CRCProposalWithdraw Stage wrong too small
	propState.WithdrawnBudgets = map[uint8]common.Fixed64{0: 10 * 1e8}
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "no need to withdraw")

	//stage =2 ok
	txn = s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 2,
		Recipient, CRCCommitteeAddressU168, 19*ela, 40*ela)
	crcProposalWithdraw, _ = txn.Payload.(*payload.CRCProposalWithdraw)
	propState.WithdrawableBudgets = map[uint8]common.Fixed64{0: 10 * 1e8, 1: 20 * 1e8}
	propState.FinalPaymentStatus = false
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//stage =3 ok
	txn = s.getCRCProposalWithdrawTx(publicKeyStr1, privateKeyStr1, 3,
		Recipient, CRCCommitteeAddressU168, 29*ela, 30*ela)
	crcProposalWithdraw, _ = txn.Payload.(*payload.CRCProposalWithdraw)
	propState.WithdrawableBudgets = map[uint8]common.Fixed64{0: 10 * 1e8, 1: 20 * 1e8, 2: 30 * 1e8}
	propState.WithdrawnBudgets = map[uint8]common.Fixed64{0: 10 * 1e8, 1: 20 * 1e8}
	propState.FinalPaymentStatus = true
	s.Chain.crCommittee.GetProposalManager().Proposals[crcProposalWithdraw.
		ProposalHash] = propState
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.NoError(err)

	//len(txn.Outputs) ==0 transaction has no outputs
	rightOutPuts := txn.Outputs
	txn.Outputs = []*types.Output{}
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	s.EqualError(err, "transaction has no outputs")

	//txn.Outputs[1].ProgramHash !=CRCComitteeAddresss
	txn.Outputs = rightOutPuts
	txn.Outputs[1].ProgramHash = *Recipient
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	txn.Outputs[1].ProgramHash = *CRCCommitteeAddressU168
	s.EqualError(err, "txn.Outputs[1].ProgramHash !=CRCComitteeAddresss")

	//len(txn.Outputs) >2 CRCProposalWithdraw tx should not have over two output
	txn.Outputs = rightOutPuts
	txn.Outputs = append(txn.Outputs, &types.Output{})
	err = s.Chain.checkTransactionOutput(txn, tenureHeight)
	s.EqualError(err, "CRCProposalWithdraw tx should not have over two output")

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	pk2Bytes, _ := common.HexStringToBytes(publicKeyStr2)

	propState.ProposalOwner = pk2Bytes
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "the OwnerPublicKey is not owner of proposal")

	references[inputs[0]] = *outputs[1]
	err = s.Chain.checkCRCProposalWithdrawTransaction(txn, references, tenureHeight)
	s.EqualError(err, "proposal withdrawal transaction for non-crc committee address")
}

func (s *txValidatorTestSuite) TestCheckCRCProposalTransaction() {
	publicKeyStr1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"

	publicKeyStr2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"

	tenureHeight := config.DefaultParams.CRCommitteeStartHeight + 1
	nickName1 := "nickname 1"

	member1 := s.getCRMember(publicKeyStr1, privateKeyStr1, nickName1)
	memebers := make(map[common.Uint168]*crstate.CRMember)
	memebers[member1.Info.DID] = member1
	s.Chain.crCommittee.Members = memebers
	s.Chain.crCommittee.CRCCommitteeBalance = common.Fixed64(100 * 1e8)
	s.Chain.crCommittee.CRCCurrentStageAmount = common.Fixed64(100 * 1e8)
	s.Chain.crCommittee.InElectionPeriod = true
	s.Chain.crCommittee.NeedAppropriation = false

	// ok
	txn := s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	err := s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.NoError(err)

	// member status is not elected
	member1.MemberState = crstate.MemberImpeached
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CR Council Member should be an elected CR members")

	// register cr proposal in voting period
	member1.MemberState = crstate.MemberElected
	tenureHeight = config.DefaultParams.CRCommitteeStartHeight +
		config.DefaultParams.CRDutyPeriod - config.DefaultParams.CRVotingPeriod
	s.Chain.crCommittee.InElectionPeriod = false
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "cr proposal tx must not during voting period")

	// recipient is empty
	s.Chain.crCommittee.InElectionPeriod = true
	tenureHeight = config.DefaultParams.CRCommitteeStartHeight + 1
	txn.Payload.(*payload.CRCProposal).Recipient = common.Uint168{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "recipient is empty")

	// invalid payload
	txn.Payload = &payload.CRInfo{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "invalid payload")

	// invalid proposal type
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).ProposalType = 0x1000
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "type of proposal should be known")

	// invalid outputs of ELIP.
	txn.Payload.(*payload.CRCProposal).ProposalType = 0x0100
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "ELIP needs to have and only have two budget")

	// invalid budgets.
	txn.Payload.(*payload.CRCProposal).ProposalType = 0x0000
	s.Chain.crCommittee.CRCCommitteeBalance = common.Fixed64(10 * 1e8)
	s.Chain.crCommittee.CRCCurrentStageAmount = common.Fixed64(10 * 1e8)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "budgets exceeds 10% of CRC committee balance")

	s.Chain.crCommittee.CRCCommitteeBalance = common.Fixed64(100 * 1e8)
	s.Chain.crCommittee.CRCCurrentStageAmount = common.Fixed64(100 * 1e8)
	s.Chain.crCommittee.CRCCommitteeUsedAmount = common.Fixed64(99 * 1e8)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "budgets exceeds the balance of CRC committee")

	s.Chain.crCommittee.CRCCommitteeUsedAmount = common.Fixed64(0)

	// CRCouncilMemberSignature is not signed by CR member
	txn = s.getCRCProposalTx(publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CR Council Member should be one of the CR members")

	// invalid owner
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).OwnerPublicKey = []byte{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "invalid owner")

	// invalid owner signature
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	txn.Payload.(*payload.CRCProposal).OwnerPublicKey = publicKey1
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "owner signature check failed")

	// invalid CR owner signature
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).CRCouncilMemberSignature = []byte{}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "failed to check CR Council Member signature")
	// proposals can not more than MaxCommitteeProposalCount
	txn = s.getCRCProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	crcProposal, _ := txn.Payload.(*payload.CRCProposal)
	proposalHashSet := crstate.NewProposalHashSet()
	for i := 0; i < int(s.Chain.chainParams.MaxCommitteeProposalCount); i++ {
		proposalHashSet.Add(*randomUint256())
	}
	s.Chain.crCommittee.GetProposalManager().ProposalHashes[crcProposal.
		CRCouncilMemberDID] = proposalHashSet
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "proposal is full")

	// invalid proposal owner
	txn = s.getCRCCloseProposalTx(publicKeyStr1, privateKeyStr1, publicKeyStr2, privateKeyStr2)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CloseProposal owner should be one of the CR members")

	// invalid closeProposalHash
	txn = s.getCRCCloseProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CloseProposalHash does not exist")

	// invalid proposal status
	publicKey2, _ := hex.DecodeString(publicKeyStr2)
	proposalState, proposal := s.createSpecificStatusProposal(publicKey1, publicKey2, tenureHeight, crstate.Registered)
	hash := proposal.Hash()
	s.Chain.crCommittee.GetProposalManager().Proposals[hash] = proposalState
	txn = s.getCRCCloseProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).CloseProposalHash = hash
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CloseProposalHash has to be voterAgreed")

	// invalid receipt
	proposalState, proposal = s.createSpecificStatusProposal(publicKey1, publicKey2, tenureHeight, crstate.VoterAgreed)
	hash = proposal.Hash()
	s.Chain.crCommittee.GetProposalManager().Proposals[hash] = proposalState
	txn = s.getCRCCloseProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).CloseProposalHash = hash
	txn.Payload.(*payload.CRCProposal).Recipient = *randomUint168()
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CloseProposal recipient must be empty")

	// invalid budget
	txn = s.getCRCCloseProposalTx(publicKeyStr2, privateKeyStr2, publicKeyStr1, privateKeyStr1)
	txn.Payload.(*payload.CRCProposal).CloseProposalHash = hash
	txn.Payload.(*payload.CRCProposal).Budgets = []payload.Budget{{
		payload.Imprest,
		0x01,
		common.Fixed64(10000000000),
	}}
	err = s.Chain.checkCRCProposalTransaction(txn, tenureHeight, 0)
	s.EqualError(err, "CloseProposal cannot have budget")

}

func (s *txValidatorTestSuite) TestCheckStringField() {
	s.NoError(checkStringField("Normal", "test", false))
	s.EqualError(checkStringField("", "test", false),
		"field test has invalid string length")
	s.EqualError(checkStringField("I am more than 100, 1234567890123456"+
		"789012345678901234567890123456789012345678901234567890123456789012345"+
		"678901234567890", "test", false), "field test"+
		" has invalid string length")
}

func (s *txValidatorTestSuite) TestCheckTransactionDepositUTXO() {
	references := make(map[*types.Input]types.Output)
	input := &types.Input{}
	var txn types.Transaction

	// Use the deposit UTXO in a TransferAsset transaction
	depositHash, _ := common.Uint168FromAddress("DVgnDnVfPVuPa2y2E4JitaWjWgRGJDuyrD")
	depositOutput := types.Output{
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
	normalOutput := types.Output{
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

func (s *txValidatorTestSuite) TestCheckReturnDepositCoinTransaction() {
	s.CurrentHeight = 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	_, pk, _ := crypto.GenerateKeyPair()
	depositCont, _ := contract.CreateDepositContractByPubKey(pk)
	publicKey, _ := pk.EncodePoint(true)
	// register CR
	s.Chain.state.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: s.CurrentHeight,
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
						Value:       common.Fixed64(5000 * 1e8),
					},
				},
			},
		},
	}, nil)
	s.CurrentHeight++
	producer := s.Chain.state.GetProducer(publicKey)
	s.True(producer.State() == state.Pending, "register producer failed")

	for i := 0; i < 6; i++ {
		s.Chain.state.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: s.CurrentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		s.CurrentHeight++
	}
	s.True(producer.State() == state.Active, "active producer failed")

	// check a return deposit coin transaction with wrong state.
	references := make(map[*types.Input]types.Output)
	references[&types.Input{}] = types.Output{
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
			Height: s.CurrentHeight,
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
	s.CurrentHeight++
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

	// check a correct return deposit coin transaction.
	rdTx.Outputs[0].Value = 4999 * 100000000
	err = s.Chain.checkReturnDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.NoError(err)
}

func (s *txValidatorTestSuite) TestCheckReturnCRDepositCoinTransaction() {
	s.CurrentHeight = 1
	_, pk, _ := crypto.GenerateKeyPair()
	cont, _ := contract.CreateStandardContract(pk)
	code := cont.Code
	depositCont, _ := contract.CreateDepositContractByPubKey(pk)
	ct, _ := contract.CreateCRIDContractByCode(code)
	cid := ct.ToProgramHash()

	s.Chain.chainParams.CRVotingStartHeight = uint32(1)
	s.Chain.chainParams.CRCommitteeStartHeight = uint32(3000)
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	// register CR
	s.Chain.crCommittee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: s.CurrentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.RegisterCR,
				Payload: &payload.CRInfo{
					Code:     code,
					CID:      *cid,
					NickName: randomString(),
				},
				Outputs: []*types.Output{
					{
						ProgramHash: *depositCont.ToProgramHash(),
						Value:       common.Fixed64(5000 * 1e8),
					},
				},
			},
		},
	}, nil)
	s.CurrentHeight++
	candidate := s.Chain.crCommittee.GetCandidate(*cid)
	s.True(candidate.State() == crstate.Pending, "register CR failed")

	for i := 0; i < 6; i++ {
		s.Chain.crCommittee.ProcessBlock(&types.Block{
			Header: types.Header{
				Height: s.CurrentHeight,
			},
			Transactions: []*types.Transaction{},
		}, nil)
		s.CurrentHeight++
	}
	s.True(candidate.State() == crstate.Active, "active CR failed")

	references := make(map[*types.Input]types.Output)
	references[&types.Input{}] = types.Output{
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
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "signer must be refundable")

	// unregister CR
	s.Chain.crCommittee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: s.CurrentHeight,
		},
		Transactions: []*types.Transaction{
			{
				TxType: types.UnregisterCR,
				Payload: &payload.UnregisterCR{
					CID: *getCID(code),
				},
			},
		},
	}, nil)
	s.CurrentHeight++
	s.True(candidate.State() == crstate.Canceled, "canceled CR failed")

	publicKey2 := "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"
	pubKeyBytes2, _ := common.HexStringToBytes(publicKey2)
	pubkey2, _ := crypto.DecodePoint(pubKeyBytes2)
	code2, _ := contract.CreateStandardRedeemScript(pubkey2)

	// check a return cr deposit coin transaction when not reached the
	// count of DepositLockupBlocks in voting period.
	rdTx.Programs[0].Code = code
	s.CurrentHeight = 2159 + canceledHeight
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2159+canceledHeight)
	s.EqualError(err, "signer must be refundable")

	s.CurrentHeight = 2160 + canceledHeight
	s.Chain.crCommittee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: s.CurrentHeight,
		},
		Transactions: []*types.Transaction{},
	}, nil)

	// check a return cr deposit coin transaction with wrong code in voting period.
	rdTx.Programs[0].Code = code2
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "signer must be candidate or member")

	// check a return cr deposit coin transaction with wrong output amount.
	rdTx.Outputs[0].Value = 5000 * 100000000
	s.CurrentHeight = 2160 + canceledHeight
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "signer must be candidate or member")

	// check a correct return cr deposit coin transaction.
	rdTx.Outputs[0].Value = 4999 * 100000000
	rdTx.Programs[0].Code = code
	s.CurrentHeight = s.Chain.chainParams.CRCommitteeStartHeight
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, s.CurrentHeight)
	s.NoError(err)

	// return CR deposit coin.
	rdTx.Programs[0].Code = code
	s.Chain.crCommittee.ProcessBlock(&types.Block{
		Header: types.Header{
			Height: s.CurrentHeight,
		},
		Transactions: []*types.Transaction{
			rdTx,
		},
	}, nil)
	s.CurrentHeight++

	// check a return cr deposit coin transaction with the amount has returned.
	err = s.Chain.checkReturnCRDepositCoinTransaction(
		rdTx, references, 2160+canceledHeight)
	s.EqualError(err, "signer must be refundable")

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

	references := make(map[*types.Input]types.Output)
	outputs := []*types.Output{{Type: types.OTNone}}
	s.NoError(s.Chain.checkVoteOutputs(0, outputs, references, nil, nil))

	publicKey1 := "02f981e4dae4983a5d284d01609ad735e3242c5672bb2c7bb0018cc36f9ab0c4a5"
	publicKey2 := "036db5984e709d2e0ec62fd974283e9a18e7b87e8403cc784baf1f61f775926535"
	publicKey3 := "031e12374bae471aa09ad479f66c2306f4bcc4ca5b754609a82a1839b94b4721b9"
	privateKeyStr1 := "15e0947580575a9b6729570bed6360a890f84a07dc837922fe92275feec837d4"
	privateKeyStr2 := "b2c25e877c8a87d54e8a20a902d27c7f24ed52810813ba175ca4e8d3036d130e"
	privateKeyStr3 := "94396a69462208b8fd96d83842855b867d3b0e663203cb31d0dfaec0362ec034"

	registerCRTxn1 := s.getRegisterCRTx(publicKey1, privateKeyStr1,
		"nickName1", payload.CRInfoVersion, &common.Uint168{})
	registerCRTxn2 := s.getRegisterCRTx(publicKey2, privateKeyStr2,
		"nickName2", payload.CRInfoVersion, &common.Uint168{})
	registerCRTxn3 := s.getRegisterCRTx(publicKey3, privateKeyStr3,
		"nickName3", payload.CRInfoVersion, &common.Uint168{})

	s.CurrentHeight = 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})
	block := &types.Block{
		Transactions: []*types.Transaction{
			registerCRTxn1,
			registerCRTxn2,
			registerCRTxn3,
		},
		Header: types.Header{Height: s.CurrentHeight},
	}
	s.Chain.crCommittee.ProcessBlock(block, nil)
	code1 := getCodeByPubKeyStr(publicKey1)
	code2 := getCodeByPubKeyStr(publicKey2)
	code3 := getCodeByPubKeyStr(publicKey3)

	candidate1, _ := common.HexStringToBytes(publicKey1)
	candidate2, _ := common.HexStringToBytes(publicKey2)
	candidateCID1 := getCID(code1)
	candidateCID2 := getCID(code2)
	candidateCID3 := getCID(code3)

	producersMap := make(map[string]struct{})
	producersMap[publicKey1] = struct{}{}
	crsMap := make(map[common.Uint168]struct{})

	crsMap[*candidateCID1] = struct{}{}
	crsMap[*candidateCID3] = struct{}{}

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
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs1, references, producersMap, crsMap),
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
						{candidateCID3.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs2, references, producersMap, crsMap),
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
						{candidateCID3.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs3, references, producersMap, crsMap),
		"the output address of vote tx should exist in its input")

	references[&types.Input{}] = types.Output{
		ProgramHash: *hash,
	}

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
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs4, references, producersMap, crsMap),
		"invalid vote output payload producer candidate: "+publicKey2)

	// Check vote output v0 with correct ouput program hash
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs1, references, producersMap, crsMap))
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs2, references, producersMap, crsMap))
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs3, references, producersMap, crsMap))

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
						{candidateCID2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs5, references, producersMap, crsMap),
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
						{candidateCID2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs6, references, producersMap, crsMap),
		"invalid vote output payload CR candidate: "+candidateCID2.String())

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
						{candidate1, 0},
					},
				},
				{
					VoteType: outputpayload.CRC,
					CandidateVotes: []outputpayload.CandidateVotes{
						{candidateCID2.Bytes(), 0},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs7, references, producersMap, crsMap),
		"payload VoteProducerVersion not support vote CR")

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
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs8, references, producersMap, crsMap),
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
						{candidateCID1.Bytes(), 10},
						{candidateCID3.Bytes(), 10},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs9, references, producersMap, crsMap),
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
						{candidateCID3.Bytes(), 20},
					},
				},
			},
		},
	})
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs10, references, producersMap, crsMap),
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
						{candidateCID3.Bytes(), 10},
					},
				},
			},
		},
	})
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs11, references, producersMap, crsMap))

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
						{candidateCID3.Bytes(), 1},
					},
				},
			},
		},
	})
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs12, references, producersMap, crsMap))

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
		&crstate.ProposalState{Status: 1}
	s.NoError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs13, references, producersMap, crsMap))

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
	s.EqualError(s.Chain.checkVoteOutputs(config.DefaultParams.CRVotingStartHeight,
		outputs14, references, producersMap, crsMap),
		"invalid CRCProposal: 9c5ab8998718e0c1c405a719542879dc7553fca05b4e89132ec8d0e88551fcc0")
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

func (s *txValidatorTestSuite) TestCreateCRCAppropriationTransaction() {
	crAddress := "ERyUmNH51roR9qfru37Kqkaok2NghR7L5U"
	crcFoundation, _ := common.Uint168FromAddress(crAddress)

	s.Chain.chainParams.CRCFoundation = *crcFoundation
	crcCommiteeAddressStr := "ESq12oQrvGqHfTkEDYJyR9MxZj1NMnonjo"

	crcCommiteeAddressHash, _ := common.Uint168FromAddress(crcCommiteeAddressStr)
	s.Chain.chainParams.CRCCommitteeAddress = *crcCommiteeAddressHash

	s.CurrentHeight = 1
	s.Chain.crCommittee = crstate.NewCommittee(s.Chain.chainParams)
	s.Chain.crCommittee.RegisterFuncitons(&crstate.CommitteeFuncsConfig{
		GetTxReference:                   s.Chain.UTXOCache.GetTxReference,
		GetUTXO:                          s.Chain.db.GetFFLDB().GetUTXO,
		GetHeight:                        func() uint32 { return s.CurrentHeight },
		CreateCRAppropriationTransaction: s.Chain.CreateCRCAppropriationTransaction,
	})

	var txOutputs []*types.Output
	txOutput := &types.Output{
		AssetID:     *elaact.SystemAssetID,
		ProgramHash: *crcFoundation,
		Value:       common.Fixed64(0),
		OutputLock:  0,
		Type:        types.OTNone,
		Payload:     &outputpayload.DefaultOutput{},
	}
	for i := 1; i < 5; i++ {
		txOutPutNew := *txOutput
		txOutPutNew.Value = common.Fixed64(i * 100)
		txOutputs = append(txOutputs, &txOutPutNew)
	}

	txn := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.TransferAsset,
		Payload:    &payload.TransferAsset{},
		Attributes: nil,
		Inputs:     nil,
		Outputs:    txOutputs,
		Programs:   nil,
		LockTime:   0,
	}

	txOutputs = nil
	txOutputCoinBase := *txOutput
	txOutputCoinBase.Value = common.Fixed64(500)
	txOutputCoinBase.OutputLock = uint32(100)
	txOutputs = append(txOutputs, &txOutputCoinBase)
	txnCoinBase := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.CoinBase,
		Payload:    &payload.TransferAsset{},
		Attributes: nil,
		Inputs:     nil,
		Outputs:    txOutputs,
		Programs:   nil,
		LockTime:   0,
	}
	block := &types.Block{
		Transactions: []*types.Transaction{
			txn,
			txnCoinBase,
		},
		Header: types.Header{
			Height:   1,
			Previous: s.Chain.chainParams.GenesisBlock.Hash(),
		},
	}
	hash := block.Hash()
	node, _ := s.Chain.LoadBlockNode(&block.Header, &hash)
	s.Chain.db.SaveBlock(block, node, nil, CalcPastMedianTime(node))
	txCrcAppropriation, _ := s.Chain.CreateCRCAppropriationTransaction()
	s.NotNil(txCrcAppropriation)
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
