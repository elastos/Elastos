package types

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"math/rand"
	"strconv"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/suite"
)

type transactionSuite struct {
	suite.Suite

	InputNum   int
	OutputNum  int
	AttrNum    int
	ProgramNum int
}

func (s *transactionSuite) SetupSuite() {
	s.InputNum = 10
	s.OutputNum = 10
	s.AttrNum = 10
	s.ProgramNum = 10
}

func (s *transactionSuite) TestCoinbaseTransaction_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(CoinBase), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadCoinBase{
		CoinbaseData: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	s.True(bytes.Equal(txn.Payload.(*payload.PayloadCoinBase).CoinbaseData, txn2.Payload.(*payload.PayloadCoinBase).CoinbaseData))
}

func (s *transactionSuite) TestRegisterAssetTransaction_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(RegisterAsset), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadRegisterAsset{
		Asset: payload.Asset{
			Name:        "test name",
			Description: "test desc",
			Precision:   byte(rand.Uint32()),
			AssetType:   payload.AssetType(rand.Uint32()),
			RecordType:  payload.AssetRecordType(rand.Uint32()),
		},
		Amount:     common.Fixed64(rand.Int63()),
		Controller: *randomUint168(),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadRegisterAsset)
	p2 := txn2.Payload.(*payload.PayloadRegisterAsset)

	s.Equal(p1.Asset.Name, p2.Asset.Name)
	s.Equal(p1.Asset.Description, p2.Asset.Description)
	s.Equal(p1.Asset.Precision, p2.Asset.Precision)
	s.Equal(p1.Asset.AssetType, p2.Asset.AssetType)
	s.Equal(p1.Asset.RecordType, p2.Asset.RecordType)
	s.Equal(p1.Amount, p2.Amount)
	s.True(p1.Controller.IsEqual(p2.Controller))
}

func (s *transactionSuite) TestTransferAssert_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(TransferAsset), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadTransferAsset{}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
}

func (s *transactionSuite) TestRecord_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(Record), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadRecord{
		RecordType: "test record type",
		RecordData: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadRecord)
	p2 := txn2.Payload.(*payload.PayloadRecord)

	s.Equal(p1.RecordType, p2.RecordType)
	s.True(bytes.Equal(p1.RecordData, p2.RecordData))
}

func (s *transactionSuite) TestSideChainPow_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(SideChainPow), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadSideChainPow{
		SideBlockHash:   *randomUint256(),
		SideGenesisHash: *randomUint256(),
		BlockHeight:     rand.Uint32(),
		SignedData:      []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadSideChainPow)
	p2 := txn2.Payload.(*payload.PayloadSideChainPow)

	s.True(p1.SideBlockHash.IsEqual(p2.SideBlockHash))
	s.True(p1.SideGenesisHash.IsEqual(p2.SideGenesisHash))
	s.Equal(p1.BlockHeight, p2.BlockHeight)
	s.True(bytes.Equal(p1.SignedData, p2.SignedData))
}

func (s *transactionSuite) TestWithdrawFromSideChain_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(WithdrawFromSideChain), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadWithdrawFromSideChain{
		BlockHeight:         rand.Uint32(),
		GenesisBlockAddress: "test genesis block address",
		SideChainTransactionHashes: []common.Uint256{
			*randomUint256(),
			*randomUint256(),
		},
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadWithdrawFromSideChain)
	p2 := txn2.Payload.(*payload.PayloadWithdrawFromSideChain)

	s.Equal(p1.BlockHeight, p2.BlockHeight)
	s.Equal(p1.GenesisBlockAddress, p2.GenesisBlockAddress)
	s.Equal(len(p1.SideChainTransactionHashes), len(p2.SideChainTransactionHashes))
	for i := range p1.SideChainTransactionHashes {
		s.True(p1.SideChainTransactionHashes[i].IsEqual(p2.SideChainTransactionHashes[i]))
	}
}

func (s *transactionSuite) TestTransferCrossChainAsset_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(TransferCrossChainAsset), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadTransferCrossChainAsset{
		CrossChainAddresses: []string{
			strconv.FormatUint(rand.Uint64(), 10),
			strconv.FormatUint(rand.Uint64(), 10),
		},
		OutputIndexes: []uint64{
			rand.Uint64(),
			rand.Uint64(),
		},
		CrossChainAmounts: []common.Fixed64{
			common.Fixed64(rand.Int63()),
			common.Fixed64(rand.Int63()),
		},
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadTransferCrossChainAsset)
	p2 := txn2.Payload.(*payload.PayloadTransferCrossChainAsset)

	s.Equal(len(p1.CrossChainAddresses), len(p2.CrossChainAddresses))
	s.Equal(len(p1.OutputIndexes), len(p2.OutputIndexes))
	s.Equal(len(p1.CrossChainAmounts), len(p2.CrossChainAmounts))
	s.Equal(len(p1.CrossChainAddresses), len(p2.OutputIndexes))
	s.Equal(len(p1.CrossChainAddresses), len(p1.CrossChainAmounts))
	for i := range p1.CrossChainAddresses {
		s.Equal(p1.CrossChainAddresses[i], p2.CrossChainAddresses[i])
	}
	for i := range p1.OutputIndexes {
		s.Equal(p1.OutputIndexes[i], p2.OutputIndexes[i])
	}
	for i := range p1.CrossChainAmounts {
		s.Equal(p1.CrossChainAmounts[i], p2.CrossChainAmounts[i])
	}
}

func (s *transactionSuite) TestRegisterProducer_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(RegisterProducer), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadRegisterProducer{
		PublicKey: []byte(strconv.FormatUint(rand.Uint64(), 10)),
		NickName:  strconv.FormatUint(rand.Uint64(), 10),
		Url:       strconv.FormatUint(rand.Uint64(), 10),
		Location:  rand.Uint64(),
		Address:   strconv.FormatUint(rand.Uint64(), 10),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadRegisterProducer)
	p2 := txn2.Payload.(*payload.PayloadRegisterProducer)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
	s.Equal(p1.NickName, p2.NickName)
	s.Equal(p1.Url, p2.Url)
	s.Equal(p1.Location, p2.Location)
	s.Equal(p1.Address, p2.Address)
}

func (s *transactionSuite) TestCancelProducer_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(CancelProducer), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadCancelProducer{
		PublicKey: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadCancelProducer)
	p2 := txn2.Payload.(*payload.PayloadCancelProducer)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
}

func (s *transactionSuite) TestUpdateProducer_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(UpdateProducer), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadUpdateProducer{
		PublicKey: []byte(strconv.FormatUint(rand.Uint64(), 10)),
		NickName:  strconv.FormatUint(rand.Uint64(), 10),
		Url:       strconv.FormatUint(rand.Uint64(), 10),
		Location:  rand.Uint64(),
		Address:   strconv.FormatUint(rand.Uint64(), 10),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.PayloadUpdateProducer)
	p2 := txn2.Payload.(*payload.PayloadUpdateProducer)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
	s.Equal(p1.NickName, p2.NickName)
	s.Equal(p1.Url, p2.Url)
	s.Equal(p1.Location, p2.Location)
	s.Equal(p1.Address, p2.Address)
}

func (s *transactionSuite) TestReturnDepositCoin_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(ReturnDepositCoin), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.PayloadReturnDepositCoin{}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
}

func (s *transactionSuite) TestTransaction_SpecificSample() {
	// update producer transaction deserialize sample
	byteReader := new(bytes.Buffer)
	updateProducerByteStr := "090B0021037F3CAEDE72447B6082C1E8F7705FFD1ED6E24F348130D34CBC7C0A35C9E993F507646F6E676C65690B7A68697A616F64616A69650C000000000000000831322E302E302E310100133331373237383934343537373235363832383201FE52A99C9CA67307BCEB50BA0A7D2E05E4461954FB34FCF29FBBEA7F7F08CB2800000000000001B037DB964A231458D2D6FFD5EA18944C4F90E63D547C5D3B9874DF66A4EAD0A300743BA40B00000000000000214FFBC4FB3B3C30A626A3B298BFA392A0121D42490000000000014140821CF72B20045AF7D29ABF9269825CE11B9BFC57BE2ED0DF71EACB61927F86238A8A022FE502DCC7F2B0FE20C854034B84AE43F65D08A4BDF5ACBA6ECF076EAD2321037F3CAEDE72447B6082C1E8F7705FFD1ED6E24F348130D34CBC7C0A35C9E993F5AC"
	updateProducerByte, _ := common.HexStringToBytes(updateProducerByteStr)
	byteReader.Write(updateProducerByte)
	txn := &Transaction{}
	s.NoError(txn.Deserialize(byteReader))
}

func TestTransactionSuite(t *testing.T) {
	suite.Run(t, new(transactionSuite))
}

func assertOldVersionTxEqual(oldVersion bool, suite *suite.Suite, first, second *Transaction, inputNum, outputNum, attrNum, programNum int) {
	if oldVersion {
		suite.Equal(TxVersionDefault, second.Version)
	} else {
		suite.Equal(first.Version, second.Version)
	}
	suite.Equal(first.TxType, second.TxType)
	suite.Equal(first.PayloadVersion, second.PayloadVersion)
	suite.Equal(first.LockTime, second.LockTime)

	suite.Equal(inputNum, len(first.Inputs))
	suite.Equal(inputNum, len(second.Inputs))
	for i := 0; i < inputNum; i++ {
		suite.Equal(first.Inputs[i].Sequence, second.Inputs[i].Sequence)
		suite.Equal(first.Inputs[i].Previous.Index, second.Inputs[i].Previous.Index)
		suite.True(first.Inputs[i].Previous.TxID.IsEqual(second.Inputs[i].Previous.TxID))
	}

	suite.Equal(outputNum, len(first.Outputs))
	suite.Equal(outputNum, len(second.Outputs))
	for i := 0; i < outputNum; i++ {
		suite.True(first.Outputs[i].AssetID.IsEqual(second.Outputs[i].AssetID))
		suite.Equal(first.Outputs[i].Value, second.Outputs[i].Value)
		suite.Equal(first.Outputs[i].OutputLock, second.Outputs[i].OutputLock)
		suite.True(first.Outputs[i].ProgramHash.IsEqual(second.Outputs[i].ProgramHash))

		if !oldVersion {
			suite.Equal(first.Outputs[i].OutputType, second.Outputs[i].OutputType)
		}
	}

	suite.Equal(attrNum, len(first.Attributes))
	suite.Equal(attrNum, len(second.Attributes))
	for i := 0; i < attrNum; i ++ {
		suite.Equal(first.Attributes[i].Usage, second.Attributes[i].Usage)
		suite.True(bytes.Equal(first.Attributes[i].Data, second.Attributes[i].Data))
	}

	suite.Equal(programNum, len(first.Programs))
	suite.Equal(programNum, len(second.Programs))
	for i := 0; i < programNum; i ++ {
		suite.True(bytes.Equal(first.Programs[i].Code, second.Programs[i].Code))
		suite.True(bytes.Equal(first.Programs[i].Parameter, second.Programs[i].Parameter))
	}
}

func randomOldVersionTransaction(oldVersion bool, txType byte, inputNum, outputNum, attrNum, programNum int) *Transaction {
	txn := &Transaction{
		Version:        TransactionVersion(txType),
		TxType:         TransactionType(txType),
		PayloadVersion: byte(rand.Uint32()),
		LockTime:       rand.Uint32(),
		Inputs:         make([]*Input, 0),
		Outputs:        make([]*Output, 0),
		Attributes:     make([]*Attribute, 0),
		Programs:       make([]*program.Program, 0),
	}
	if !oldVersion {
		txn.Version = TxVersion09
	}

	for i := 0; i < inputNum; i++ {
		txn.Inputs = append(txn.Inputs, &Input{
			Sequence: rand.Uint32(),
			Previous: OutPoint{
				TxID:  *randomUint256(),
				Index: uint16(rand.Uint32()),
			},
		})
	}

	for i := 0; i < outputNum; i++ {
		output := &Output{
			AssetID:       *randomUint256(),
			Value:         common.Fixed64(rand.Int63()),
			OutputLock:    rand.Uint32(),
			ProgramHash:   *randomUint168(),
			OutputType:    0,
			OutputPayload: nil,
		}
		if !oldVersion {
			output.OutputType = DefaultOutput
			output.OutputPayload = &outputpayload.DefaultOutput{}
		}
		txn.Outputs = append(txn.Outputs, output)
	}

	validAttrUsage := []AttributeUsage{Nonce, Script, Memo, Description, DescriptionUrl, Confirmations}
	for i := 0; i < attrNum; i++ {
		txn.Attributes = append(txn.Attributes, &Attribute{
			Usage: validAttrUsage[rand.Intn(len(validAttrUsage))],
			Data:  []byte(strconv.FormatUint(rand.Uint64(), 10)),
		})
	}

	for i := 0; i < programNum; i++ {
		txn.Programs = append(txn.Programs, &program.Program{
			Code:      []byte(strconv.FormatUint(rand.Uint64(), 10)),
			Parameter: []byte(strconv.FormatUint(rand.Uint64(), 10)),
		})
	}

	return txn
}

func randomUint256() *common.Uint256 {
	randBytes := make([]byte, 32)
	rand.Read(randBytes)

	result, _ := common.Uint256FromBytes(randBytes)
	return result
}

func randomUint168() *common.Uint168 {
	randBytes := make([]byte, 21)
	rand.Read(randBytes)
	result, _ := common.Uint168FromBytes(randBytes)

	return result
}
