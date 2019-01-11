package types

import (
	"bytes"
	"math/rand"
	"strconv"
	"testing"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
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
	txn.Payload = &payload.CoinBase{
		data: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	s.True(bytes.Equal(txn.Payload.(*payload.CoinBase).data, txn2.Payload.(*payload.CoinBase).data))
}

func (s *transactionSuite) TestRegisterAssetTransaction_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(RegisterAsset), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.RegisterAsset{
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

	p1 := txn.Payload.(*payload.RegisterAsset)
	p2 := txn2.Payload.(*payload.RegisterAsset)

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
	txn.Payload = &payload.TransferAsset{}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
}

func (s *transactionSuite) TestRecord_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(Record), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.Record{
		Type:    "test record type",
		Content: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(true, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.Record)
	p2 := txn2.Payload.(*payload.Record)

	s.Equal(p1.Type, p2.Type)
	s.True(bytes.Equal(p1.Content, p2.Content))
}

func (s *transactionSuite) TestSideChainPow_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(SideChainPow), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.SideChainPow{
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

	p1 := txn.Payload.(*payload.SideChainPow)
	p2 := txn2.Payload.(*payload.SideChainPow)

	s.True(p1.SideBlockHash.IsEqual(p2.SideBlockHash))
	s.True(p1.SideGenesisHash.IsEqual(p2.SideGenesisHash))
	s.Equal(p1.BlockHeight, p2.BlockHeight)
	s.True(bytes.Equal(p1.SignedData, p2.SignedData))
}

func (s *transactionSuite) TestWithdrawFromSideChain_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(WithdrawFromSideChain), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.WithdrawFromSideChain{
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

	p1 := txn.Payload.(*payload.WithdrawFromSideChain)
	p2 := txn2.Payload.(*payload.WithdrawFromSideChain)

	s.Equal(p1.BlockHeight, p2.BlockHeight)
	s.Equal(p1.GenesisBlockAddress, p2.GenesisBlockAddress)
	s.Equal(len(p1.SideChainTransactionHashes), len(p2.SideChainTransactionHashes))
	for i := range p1.SideChainTransactionHashes {
		s.True(p1.SideChainTransactionHashes[i].IsEqual(p2.SideChainTransactionHashes[i]))
	}
}

func (s *transactionSuite) TestTransferCrossChainAsset_SerializeDeserialize() {
	txn := randomOldVersionTransaction(true, byte(TransferCrossChainAsset), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.TransferCrossChainAsset{
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

	p1 := txn.Payload.(*payload.TransferCrossChainAsset)
	p2 := txn2.Payload.(*payload.TransferCrossChainAsset)

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
	txn.Payload = &payload.ProducerInfo{
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

	p1 := txn.Payload.(*payload.ProducerInfo)
	p2 := txn2.Payload.(*payload.ProducerInfo)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
	s.Equal(p1.NickName, p2.NickName)
	s.Equal(p1.Url, p2.Url)
	s.Equal(p1.Location, p2.Location)
	s.Equal(p1.Address, p2.Address)
}

func (s *transactionSuite) TestCancelProducer_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(CancelProducer), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.CancelProducer{
		PublicKey: []byte(strconv.FormatUint(rand.Uint64(), 10)),
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	p1 := txn.Payload.(*payload.CancelProducer)
	p2 := txn2.Payload.(*payload.CancelProducer)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
}

func (s *transactionSuite) TestUpdateProducer_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(UpdateProducer), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.ProducerInfo{
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

	p1 := txn.Payload.(*payload.ProducerInfo)
	p2 := txn2.Payload.(*payload.ProducerInfo)

	s.True(bytes.Equal(p1.PublicKey, p2.PublicKey))
	s.Equal(p1.NickName, p2.NickName)
	s.Equal(p1.Url, p2.Url)
	s.Equal(p1.Location, p2.Location)
	s.Equal(p1.Address, p2.Address)
}

func (s *transactionSuite) TestReturnDepositCoin_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(ReturnDepositCoin), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &payload.ReturnDepositCoin{}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
}

func (s *transactionSuite) TestIllegalProposalEvidence_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(IllegalProposalEvidence), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &PayloadIllegalProposal{
		DposIllegalProposals: DposIllegalProposals{
			Evidence: ProposalEvidence{
				Proposal: DPosProposal{
					Sponsor:    strconv.FormatUint(rand.Uint64(), 10),
					BlockHash:  *randomUint256(),
					ViewOffset: rand.Uint32(),
					Sign:       []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
				BlockHeader: *randomBlockHeader(),
			},
			CompareEvidence: ProposalEvidence{
				Proposal: DPosProposal{
					Sponsor:    strconv.FormatUint(rand.Uint64(), 10),
					BlockHash:  *randomUint256(),
					ViewOffset: rand.Uint32(),
					Sign:       []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
				BlockHeader: *randomBlockHeader(),
			},
		},
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	s.True(txn.Payload.(*PayloadIllegalProposal).Hash().IsEqual(txn2.Payload.(*PayloadIllegalProposal).Hash()))
}

func (s *transactionSuite) TestIllegalVoteEvidence_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(IllegalVoteEvidence), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &PayloadIllegalVote{
		DposIllegalVotes: DposIllegalVotes{
			Evidence: VoteEvidence{
				Proposal: DPosProposal{
					Sponsor:    strconv.FormatUint(rand.Uint64(), 10),
					BlockHash:  *randomUint256(),
					ViewOffset: rand.Uint32(),
					Sign:       []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
				BlockHeader: *randomBlockHeader(),
				Vote: DPosProposalVote{
					ProposalHash: *randomUint256(),
					Signer:       strconv.FormatUint(rand.Uint64(), 10),
					Accept:       true,
					Sign:         []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
			},
			CompareEvidence: VoteEvidence{
				Proposal: DPosProposal{
					Sponsor:    strconv.FormatUint(rand.Uint64(), 10),
					BlockHash:  *randomUint256(),
					ViewOffset: rand.Uint32(),
					Sign:       []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
				BlockHeader: *randomBlockHeader(),
				Vote: DPosProposalVote{
					ProposalHash: *randomUint256(),
					Signer:       strconv.FormatUint(rand.Uint64(), 10),
					Accept:       true,
					Sign:         []byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
			},
		},
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	s.True(txn.Payload.(*PayloadIllegalVote).Hash().IsEqual(txn2.Payload.(*PayloadIllegalVote).Hash()))
}

func (s *transactionSuite) TestIllegalBlockEvidence_SerializeDeserialize() {
	txn := randomOldVersionTransaction(false, byte(IllegalBlockEvidence), s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)
	txn.Payload = &PayloadIllegalBlock{
		DposIllegalBlocks: DposIllegalBlocks{
			CoinType:    CoinType(rand.Uint32()),
			BlockHeight: rand.Uint32(),
			Evidence: BlockEvidence{
				Block:        []byte(strconv.FormatUint(rand.Uint64(), 10)),
				BlockConfirm: []byte(strconv.FormatUint(rand.Uint64(), 10)),
				Signers: [][]byte{
					[]byte(strconv.FormatUint(rand.Uint64(), 10)),
					[]byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
			},
			CompareEvidence: BlockEvidence{
				Block:        []byte(strconv.FormatUint(rand.Uint64(), 10)),
				BlockConfirm: []byte(strconv.FormatUint(rand.Uint64(), 10)),
				Signers: [][]byte{
					[]byte(strconv.FormatUint(rand.Uint64(), 10)),
					[]byte(strconv.FormatUint(rand.Uint64(), 10)),
				},
			},
		},
	}

	serializedData := new(bytes.Buffer)
	txn.Serialize(serializedData)

	txn2 := &Transaction{}
	txn2.Deserialize(serializedData)

	assertOldVersionTxEqual(false, &s.Suite, txn, txn2, s.InputNum, s.OutputNum, s.AttrNum, s.ProgramNum)

	s.True(txn.Payload.(*PayloadIllegalBlock).Hash().IsEqual(txn2.Payload.(*PayloadIllegalBlock).Hash()))
}

func (s *transactionSuite) TestTransaction_SpecificSample() {
	// update producer transaction deserialize sample
	byteReader := new(bytes.Buffer)
	updateProducerByteStr := "090b0021034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16c09656c615f74657374310d656c615f74657374312e6f726754b60100000000000931302e31302e302e3240a85aedec5444f8603788a0720f98e860168afcb0e82168cd1a29995a8fffba23d4ef1487b6eee4b145a660e017a5e9401c285550d2deac44b968a17c954465a600018dc8831894f8d5bcecd4b03848900d72ed756380a5bc1940f2b7368b5e3919090100ffffffff01b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300d414000000000000000000210d4109bf00e6d782db40ab183491c03cf4d6a37a00000000000141404c214698b25a1e23b5c933eb3a9f05ed83ae5fd36f8f82a90462a1d06eaac656c303f8aa1817ba7b18b5de21df1d504c86e4cc5113081bfded4088dcef2011852321034f3a7d2f33ac7f4e30876080d359ce5f314c9eabddbaaca637676377f655e16cac"
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
	for i := 0; i < attrNum; i++ {
		suite.Equal(first.Attributes[i].Usage, second.Attributes[i].Usage)
		suite.True(bytes.Equal(first.Attributes[i].Data, second.Attributes[i].Data))
	}

	suite.Equal(programNum, len(first.Programs))
	suite.Equal(programNum, len(second.Programs))
	for i := 0; i < programNum; i++ {
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

func randomBlockHeader() *Header {
	return &Header{
		Version:    rand.Uint32(),
		Previous:   *randomUint256(),
		MerkleRoot: *randomUint256(),
		Timestamp:  rand.Uint32(),
		Bits:       rand.Uint32(),
		Nonce:      rand.Uint32(),
		Height:     rand.Uint32(),
		AuxPow: auxpow.AuxPow{
			AuxMerkleBranch: []common.Uint256{
				*randomUint256(),
				*randomUint256(),
			},
			AuxMerkleIndex: rand.Int(),
			ParCoinbaseTx: auxpow.BtcTx{
				Version: rand.Int31(),
				TxIn: []*auxpow.BtcTxIn{
					{
						PreviousOutPoint: auxpow.BtcOutPoint{
							Hash:  *randomUint256(),
							Index: rand.Uint32(),
						},
						SignatureScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
						Sequence:        rand.Uint32(),
					},
					{
						PreviousOutPoint: auxpow.BtcOutPoint{
							Hash:  *randomUint256(),
							Index: rand.Uint32(),
						},
						SignatureScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
						Sequence:        rand.Uint32(),
					},
				},
				TxOut: []*auxpow.BtcTxOut{
					{
						Value:    rand.Int63(),
						PkScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
					},
					{
						Value:    rand.Int63(),
						PkScript: []byte(strconv.FormatUint(rand.Uint64(), 10)),
					},
				},
				LockTime: rand.Uint32(),
			},
			ParCoinBaseMerkle: []common.Uint256{
				*randomUint256(),
				*randomUint256(),
			},
			ParMerkleIndex: rand.Int(),
			ParBlockHeader: auxpow.BtcHeader{
				Version:    rand.Uint32(),
				Previous:   *randomUint256(),
				MerkleRoot: *randomUint256(),
				Timestamp:  rand.Uint32(),
				Bits:       rand.Uint32(),
				Nonce:      rand.Uint32(),
			},
			ParentHash: *randomUint256(),
		},
	}
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
