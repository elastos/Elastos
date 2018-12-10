package blockchain

import (
	"bytes"
	"crypto/elliptic"
	"crypto/rand"
	"fmt"
	"math"
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/version/heights"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/stretchr/testify/assert"
)

var ELA = int64(math.Pow(10, 8))

func TestTxValidatorInit(t *testing.T) {
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
	FoundationAddress = *foundation
	chainStore, err := NewTestChainStore()
	if err != nil {
		log.Error(err)
		os.Exit(-1)
	}

	err = Init(chainStore, &blockHeightMock{})
	if err != nil {
		log.Error(err)
		os.Exit(-1)
	}
}

func TestCheckTransactionSize(t *testing.T) {
	tx := buildTx()
	buf := new(bytes.Buffer)
	err := tx.Serialize(buf)
	if !assert.NoError(t, err) {
		return
	}

	size := tx.GetSize()
	// normal
	config.Parameters.MaxBlockSize = size
	err = CheckTransactionSize(tx)
	assert.NoError(t, err, "[CheckTransactionSize] passed normal size")

	// invalid
	config.Parameters.MaxBlockSize = size - 1
	err = CheckTransactionSize(tx)
	assert.EqualError(t, err, fmt.Sprintf("Invalid transaction size: %d bytes", size))

	t.Log("[TestCheckTransactionSize] PASSED")
}

func TestCheckTransactionInput(t *testing.T) {
	// coinbase transaction
	tx := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	tx.Inputs[0].Previous.Index = math.MaxUint16
	err := CheckTransactionInput(tx)
	assert.NoError(t, err)

	// invalid coinbase refer index
	tx.Inputs[0].Previous.Index = 0
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "invalid coinbase input")

	// invalid coinbase refer id
	tx.Inputs[0].Previous.Index = math.MaxUint16
	rand.Read(tx.Inputs[0].Previous.TxID[:])
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "invalid coinbase input")

	// multiple coinbase inputs
	tx.Inputs = append(tx.Inputs, &types.Input{})
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "coinbase must has only one input")

	// normal transaction
	tx = buildTx()
	err = CheckTransactionInput(tx)
	assert.NoError(t, err)

	// no inputs
	tx.Inputs = nil
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "transaction has no inputs")

	// normal transaction with coinbase input
	tx.Inputs = append(tx.Inputs, &types.Input{Previous: *types.NewOutPoint(common.EmptyHash, math.MaxUint16)})
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "invalid transaction input")

	// duplicated inputs
	tx = buildTx()
	tx.Inputs = append(tx.Inputs, tx.Inputs[0])
	err = CheckTransactionInput(tx)
	assert.EqualError(t, err, "duplicated transaction inputs")

	t.Log("[TestCheckTransactionInput] PASSED")
}

func TestCheckTransactionOutput(t *testing.T) {
	// coinbase
	tx := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
	}
	err := CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.NoError(t, err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: FoundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: FoundationAddress},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "Asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30% (CheckTxOut version)
	totalReward := RewardAmountPerBlock
	t.Logf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.NoError(t, err)

	// reward to foundation in coinbase < 30% (CheckTxOut version)
	foundationReward = common.Fixed64(float64(totalReward) * 0.2999999)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: totalReward - foundationReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "Reward to foundation in coinbase < 30%")

	// reward to foundation in coinbase = 30%, reward to miner in coinbase >= 35%
	foundationReward = common.Fixed64(float64(totalReward) * 0.3)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward := common.Fixed64(float64(totalReward) * 0.35)
	t.Logf("Miner reward amount %s", minerReward.String())
	dposReward := totalReward - foundationReward - minerReward
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: dposReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.NoError(t, err)

	// reward to foundation in coinbase = 30%, reward to miner in coinbase < 35%
	//fixme
	//foundationReward = common.Fixed64(float64(totalReward) * 0.3)
	//t.Logf("Foundation reward amount %s", foundationReward.String())
	//minerReward = common.Fixed64(float64(totalReward) * 0.3499999)
	//t.Logf("Miner reward amount %s", minerReward.String())
	//dposReward = totalReward - foundationReward - minerReward
	//tx.Outputs = []*types.Output{
	//	{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
	//	{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
	//	{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: dposReward},
	//}
	//err = CheckTransactionOutput(heights.HeightVersion1, tx)
	//assert.EqualError(t, err, "Reward to dpos in coinbase < 35%")

	// reward to foundation in coinbase < 30%, reward to miner in coinbase >= 35%
	foundationReward = common.Fixed64(float64(totalReward) * 0.2999999)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward = common.Fixed64(float64(totalReward) * 0.35)
	t.Logf("Miner reward amount %s", minerReward.String())
	dposReward = totalReward - foundationReward - minerReward
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: dposReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "Reward to foundation in coinbase < 30%")

	// reward to foundation in coinbase < 30%, reward to miner in coinbase < 35%
	foundationReward = common.Fixed64(float64(totalReward) * 0.2999999)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward = common.Fixed64(float64(totalReward) * 0.3499999)
	t.Logf("Miner reward amount %s", minerReward.String())
	dposReward = totalReward - foundationReward - minerReward
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: dposReward},
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "Reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.NoError(t, err)

	// outputs < 1
	tx.Outputs = nil
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "asset ID in output is invalid")

	// invalid program hash
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
		address := common.Uint168{}
		address[0] = 0x23
		output.ProgramHash = address
	}
	//fixme
	//err = CheckTransactionOutput(heights.HeightVersion1, tx)
	//assert.EqualError(t, err, "output address is invalid")

	t.Log("[TestCheckTransactionOutput] PASSED")
}

func TestCheckAssetPrecision(t *testing.T) {
	// normal transaction
	tx := buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
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
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.PersistAsset(register.Hash(), asset)
	DefaultLedger.Store.(*ChainStore).BatchCommit()

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

	t.Log("[TestCheckAssetPrecision] PASSED")
}

func TestCheckAmountPrecision(t *testing.T) {
	// precision check
	for i := 8; i >= 0; i-- {
		amount := common.Fixed64(math.Pow(10, float64(i)))
		t.Logf("Amount %s", amount.String())
		assert.Equal(t, true, checkAmountPrecise(amount, byte(8-i)))
		assert.Equal(t, false, checkAmountPrecise(amount, byte(8-i-1)))
	}
	t.Log("[TestCheckAmountPrecision] PASSED")
}

func TestCheckAttributeProgram(t *testing.T) {
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
	assert.EqualError(t, err, "no programs found in transaction")

	// invalid attributes
	getInvalidUsage := func() types.AttributeUsage {
		var usage = make([]byte, 1)
	NEXT:
		rand.Read(usage)
		for _, u := range usages {
			if u == types.AttributeUsage(usage[0]) {
				goto NEXT
			}
		}
		return types.AttributeUsage(usage[0])
	}
	for i := 0; i < 10; i++ {
		attr := types.NewAttribute(getInvalidUsage(), nil)
		tx.Attributes = []*types.Attribute{&attr}
		err := CheckAttributeProgram(heights.HeightVersion1, tx)
		assert.EqualError(t, err, fmt.Sprintf("invalid attribute usage %v", attr.Usage))
	}
	tx.Attributes = nil

	// empty programs
	tx.Programs = []*program.Program{}
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "no programs found in transaction")

	// nil program code
	p := &program.Program{}
	tx.Programs = append(tx.Programs, p)
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "invalid program code nil")

	// nil program parameter
	var code = make([]byte, 21)
	rand.Read(code)
	p = &program.Program{Code: code}
	tx.Programs = []*program.Program{p}
	err = CheckAttributeProgram(heights.HeightVersion1, tx)
	assert.EqualError(t, err, "invalid program parameter nil")

	// invalid program code
	getInvalidCode := func() []byte {
		var code = make([]byte, 21)
	NEXT:
		rand.Read(code)
		switch code[len(code)-1] {
		case common.STANDARD, common.MULTISIG, common.CROSSCHAIN:
			goto NEXT
		}
		return code
	}
	for i := 0; i < 10; i++ {
		p = &program.Program{Code: getInvalidCode(), Parameter: make([]byte, 1)}
		tx.Programs = []*program.Program{p}
		err = CheckAttributeProgram(heights.HeightVersion1, tx)
		assert.EqualError(t, err, fmt.Sprintf("invalid program code type, %x", p.Code))
	}

	t.Log("[TestCheckAttributeProgram] PASSED")
}

func TestCheckTransactionPayload(t *testing.T) {
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
	assert.NoError(t, err)

	// invalid precision
	payload.Asset.Precision = 9
	err = CheckTransactionPayload(tx)
	assert.EqualError(t, err, "Invalide asset Precision.")

	// invalid amount
	payload.Asset.Precision = 0
	payload.Amount = 1234567
	err = CheckTransactionPayload(tx)
	assert.EqualError(t, err, "Invalide asset value,out of precise.")

	t.Log("[TestCheckTransactionPayload] PASSED")
}

func TestCheckDuplicateSidechainTx(t *testing.T) {
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
	assert.EqualError(t, err, "Duplicate sidechain tx detected in a transaction")

	t.Log("[TestCheckDuplicateSidechainTx] PASSED")
}

func TestCheckTransactionBalance(t *testing.T) {
	// WithdrawFromSideChain will pass check in any condition
	tx := new(types.Transaction)
	tx.TxType = types.WithdrawFromSideChain
	references, _ := DefaultLedger.Store.GetTxReference(tx)
	var err error
	// deposit 100 ELA to foundation account
	deposit := NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	deposit.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(100 * ELA)},
	}
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).PersistTransaction(deposit, 0)
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	// // invalid output value
	tx = NewCoinBaseTransaction(new(payload.PayloadCoinBase), 0)
	tx.Inputs = []*types.Input{
		{Previous: *types.NewOutPoint(deposit.Hash(), 0)},
	}
	//tx.Outputs = []*types.Output{
	//	{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(-20 * ELA)},
	//	{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(-60 * ELA)},
	//}
	//references, _ = DefaultLedger.Store.GetTxReference(tx)
	//err = CheckTransactionFee(tx, references)
	//assert.EqualError(t, err, "Invalide transaction UTXO output.")

	// invalid transaction fee
	config.Parameters.PowConfiguration.MinTxFee = int(1 * ELA)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(30 * ELA)},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(70 * ELA)},
	}
	references, _ = DefaultLedger.Store.GetTxReference(tx)
	err = CheckTransactionFee(tx, references)
	assert.EqualError(t, err, "transaction fee not enough")

	// rollback deposit above
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).RollbackTransaction(deposit)
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	t.Log("[TestCheckTransactionBalance] PASSED")
}

func TestCheckSideChainPowConsensus(t *testing.T) {
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
	err := CheckSideChainPowConsensus(txn, arbitrator1)
	if err != nil {
		t.Error("TestCheckSideChainPowConsensus failed.")
	}

	err = CheckSideChainPowConsensus(txn, arbitrator2)
	if err == nil {
		t.Error("TestCheckSideChainPowConsensus failed.")
	}
}

func TestCheckDestructionAddress(t *testing.T) {
	destructionAddress := "ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr"
	txID, _ := common.Uint256FromHexString("7e8863a503e90e6464529feb1c25d98c903e01bec00ccfea2475db4e37d7328b")
	programHash, _ := common.Uint168FromAddress(destructionAddress)
	reference := map[*types.Input]*types.Output{
		&types.Input{Previous: types.OutPoint{*txID, 1234}, Sequence: 123456}: &types.Output{ProgramHash: *programHash},
	}

	err := CheckDestructionAddress(reference)
	assert.EqualError(t, err, fmt.Sprintf("cannot use utxo in the Elastos foundation destruction address"))
}

func getCode(publicKey string) []byte {
	pkBytes, _ := common.HexStringToBytes(publicKey)
	pk, _ := crypto.DecodePoint(pkBytes)
	redeemScript, _ := crypto.CreateStandardRedeemScript(pk)
	return redeemScript
}

func TestCheckRegisterProducerTransaction(t *testing.T) {
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

	publicKeyPlege1, _ := contract.PublicKeyToPledgeProgramHash(publicKey1)
	txn.Outputs = []*types.Output{&types.Output{
		AssetID:     common.Uint256{},
		Value:       5000,
		OutputLock:  0,
		ProgramHash: *publicKeyPlege1,
	}}

	// 2. Check transaction
	err := CheckRegisterProducerTransaction(txn)
	assert.NoError(t, err)

	// 3. Change public key in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = errPublicKey

	// 4. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid publick key.")

	// 5. Change public key in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = publicKey2

	// 6. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Public key unsigned.")

	// 7. Change url in payload
	txn.Payload.(*payload.PayloadRegisterProducer).PublicKey = publicKey1
	txn.Payload.(*payload.PayloadRegisterProducer).Url = ""

	// 8. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid url.")

	// 9. Persist transaction
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).PersistRegisterProducer(txn.Payload.(*payload.PayloadRegisterProducer))
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	// 10. Check transaction
	err = CheckRegisterProducerTransaction(txn)
	assert.EqualError(t, err, "Duplicated public key.")

	t.Log("[TestCheckRegisterProducerTransaction] PASSED")
}

func TestCheckVoteProducerOutput(t *testing.T) {
	// 1. Generate a vote output
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)

	programHash1, _ := contract.PublicKeyToStandardProgramHash(publicKey1)
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
						Candidates: []common.Uint168{
							*programHash1,
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
						Candidates: []common.Uint168{},
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
						Candidates: []common.Uint168{
							*programHash1,
							*programHash1,
						},
					},
				},
			},
		},
	}

	// 2. Check output payload
	err := outputs[0].OutputPayload.(*outputpayload.VoteOutput).Validate()
	assert.NoError(t, err)

	err = outputs[1].OutputPayload.(*outputpayload.VoteOutput).Validate()
	assert.EqualError(t, err, "invalid public key length.")

	err = outputs[2].OutputPayload.(*outputpayload.VoteOutput).Validate()
	assert.EqualError(t, err, "duplicate candidate")

	t.Log("[TestCheckVoteProducerOutput] PASSED")
}

func TestCheckCancelProducerTransaction(t *testing.T) {
	// 1. Generate a cancel producer transaction
	publicKeyStr1 := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd47e944507292ea08dd"
	publicKey1, _ := common.HexStringToBytes(publicKeyStr1)
	publicKeyStr2 := "027c4f35081821da858f5c7197bac5e33e77e5af4a3551285f8a8da0a59bd37c45"
	publicKey2, _ := common.HexStringToBytes(publicKeyStr2)
	errPublicKeyStr := "02b611f07341d5ddce51b5c4366aca7b889cfe0993bd63fd4"
	errPublicKey, _ := common.HexStringToBytes(errPublicKeyStr)

	txn := new(types.Transaction)
	txn.TxType = types.CancelProducer
	txn.Payload = &payload.PayloadCancelProducer{
		PublicKey: publicKey1,
	}

	txn.Programs = []*program.Program{&program.Program{
		Code:      getCode(publicKeyStr1),
		Parameter: nil,
	}}

	// 2. Check transaction
	err := CheckCancelProducerTransaction(txn)
	assert.NoError(t, err)

	// 3. Change public key in payload
	txn.Payload.(*payload.PayloadCancelProducer).PublicKey = errPublicKey

	// 4. Check transaction
	err = CheckCancelProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid publick key.")

	// 5. Change public key in payload
	txn.Payload.(*payload.PayloadCancelProducer).PublicKey = publicKey2

	// 6. Check transaction
	err = CheckCancelProducerTransaction(txn)
	assert.EqualError(t, err, "Public key unsigned.")

	// 7. Persist transaction
	txn.Payload.(*payload.PayloadCancelProducer).PublicKey = publicKey1
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).PersistCancelProducer(txn.Payload.(*payload.PayloadCancelProducer))
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	// 8. Check transaction
	err = CheckCancelProducerTransaction(txn)
	assert.EqualError(t, err, "Invalid producer.")

	t.Log("[TestCheckCancelProducerTransaction] PASSED")
}

func TestTxValidatorDone(t *testing.T) {
	DefaultLedger.Store.Close()
}
