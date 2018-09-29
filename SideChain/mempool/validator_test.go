package mempool

import (
	"bytes"
	"crypto/rand"
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/stretchr/testify/assert"
)

var ELA = int64(math.Pow(10, 8))

func TestTxValidatorInit(t *testing.T) {
	foundation, err := common.Uint168FromAddress("8VYXVxKKSAxkmRrfmGpQR2Kc66XhG6m3ta")
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	FoundationAddress = *foundation
	chainStore, err := newTestChainStore()
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	err = Init(chainStore)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
}

func TestCheckTransactionSize(t *testing.T) {
	tx := buildTx()
	buf := new(bytes.Buffer)
	err := tx.Serialize(buf)
	if !assert.NoError(t, err) {
		t.FailNow()
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

func TestCheckOutputProgramHash(t *testing.T) {
	programHash := common.Uint168{}

	// empty program hash should pass
	assert.Equal(t, true, CheckOutputProgramHash(programHash))

	// prefix standard program hash should pass
	programHash[0] = common.PrefixStandard
	assert.Equal(t, true, CheckOutputProgramHash(programHash))

	// prefix multisig program hash should pass
	programHash[0] = common.PrefixMultisig
	assert.Equal(t, true, CheckOutputProgramHash(programHash))

	// prefix crosschain program hash should pass
	programHash[0] = common.PrefixCrossChain
	assert.Equal(t, true, CheckOutputProgramHash(programHash))

	// other prefix program hash should not pass
	programHash[0] = 0x34
	assert.Equal(t, false, CheckOutputProgramHash(programHash))

	t.Log("[TestCheckOutputProgramHash] PASSED")
}

func TestCheckTransactionInput(t *testing.T) {
	// coinbase transaction
	tx := NewCoinBaseTransaction(new(types.PayloadCoinBase), 0)
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
	tx := NewCoinBaseTransaction(new(types.PayloadCoinBase), 0)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress},
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress},
	}
	err := CheckTransactionOutput(tx)
	assert.NoError(t, err)

	// outputs < 2
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress},
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*types.Output{
		{AssetID: common.EmptyHash, ProgramHash: FoundationAddress},
		{AssetID: common.EmptyHash, ProgramHash: FoundationAddress},
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "asset ID in coinbase is invalid")

	// reward to foundation in coinbase = 30%
	totalReward := common.Fixed64(1 * ELA)
	t.Logf("Block reward amount %s", totalReward.String())
	foundationReward := common.Fixed64(float64(totalReward) * 0.3)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward := totalReward - foundationReward
	t.Logf("Miner reward amount %s", minerReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultChain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
	}
	err = CheckTransactionOutput(tx)
	assert.NoError(t, err)

	// reward to foundation in coinbase < 30%
	foundationReward = common.Fixed64(float64(totalReward) * 0.2999999)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward = totalReward - foundationReward
	t.Logf("Miner reward amount %s", minerReward.String())
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultChain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "Reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultChain.AssetID
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(tx)
	assert.NoError(t, err)

	// outputs < 1
	tx.Outputs = nil
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "transaction has no outputs")

	// invalid asset ID
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = common.EmptyHash
		output.ProgramHash = common.Uint168{}
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "asset ID in output is invalid")

	// invalid program hash
	tx.Outputs = randomOutputs()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultChain.AssetID
		address := common.Uint168{}
		address[0] = 0x23
		output.ProgramHash = address
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "output address is invalid")

	t.Log("[TestCheckTransactionOutput] PASSED")
}

func TestCheckAssetPrecision(t *testing.T) {
	// normal transaction
	tx := buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultChain.AssetID
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
	asset := types.Asset{
		Name:      "TEST",
		Precision: 0x04,
		AssetType: 0x00,
	}
	register := &types.Transaction{
		TxType:         types.RegisterAsset,
		PayloadVersion: 0,
		Payload: &types.PayloadRegisterAsset{
			Asset:  asset,
			Amount: 0 * 100000000,
		},
	}
	DefaultChain.(*ChainStore).NewBatch()
	DefaultChain.PersistAsset(register.Hash(), asset)
	DefaultChain.(*ChainStore).BatchCommit()

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
	err := CheckAttributeProgram(tx)
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
		err := CheckAttributeProgram(tx)
		assert.EqualError(t, err, fmt.Sprintf("invalid attribute usage %v", attr.Usage))
	}
	tx.Attributes = nil

	// empty programs
	tx.Programs = []*types.Program{}
	err = CheckAttributeProgram(tx)
	assert.EqualError(t, err, "no programs found in transaction")

	// nil program code
	program := &types.Program{}
	tx.Programs = append(tx.Programs, program)
	err = CheckAttributeProgram(tx)
	assert.EqualError(t, err, "invalid program code nil")

	// nil program parameter
	var code = make([]byte, 21)
	rand.Read(code)
	program = &types.Program{Code: code}
	tx.Programs = []*types.Program{program}
	err = CheckAttributeProgram(tx)
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
		program = &types.Program{Code: getInvalidCode(), Parameter: make([]byte, 1)}
		tx.Programs = []*types.Program{program}
		err = CheckAttributeProgram(tx)
		assert.EqualError(t, err, fmt.Sprintf("invalid program code %x", program.Code))
	}

	t.Log("[TestCheckAttributeProgram] PASSED")
}

func TestCheckTransactionPayload(t *testing.T) {
	// normal
	tx := new(types.Transaction)
	payload := &types.PayloadRegisterAsset{
		Asset: types.Asset{
			Name:      "ELA",
			Precision: 0x08,
			AssetType: types.Token,
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

func TestCheckTransactionBalance(t *testing.T) {
	// WithdrawFromSideChain will pass check in any condition
	tx := new(types.Transaction)
	tx.TxType = types.WithdrawFromSideChain
	err := CheckTransactionBalance(tx)
	assert.NoError(t, err)

	// deposit 100 ELA to foundation account
	deposit := NewCoinBaseTransaction(new(types.PayloadCoinBase), 0)
	deposit.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(100 * ELA)},
	}
	DefaultChain.(*ChainStore).NewBatch()
	DefaultChain.(*ChainStore).PersistTransaction(deposit, 0)
	DefaultChain.(*ChainStore).BatchCommit()

	// invalid output value
	tx = NewCoinBaseTransaction(new(types.PayloadCoinBase), 0)
	tx.Inputs = []*types.Input{
		{Previous: *types.NewOutPoint(deposit.Hash(), 0)},
	}
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(-20 * ELA)},
		{AssetID: DefaultChain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(-60 * ELA)},
	}
	err = CheckTransactionBalance(tx)
	assert.EqualError(t, err, "Invalide transaction UTXO output.")

	// invalid transaction fee
	config.Parameters.PowConfiguration.MinTxFee = int(1 * ELA)
	tx.Outputs = []*types.Output{
		{AssetID: DefaultChain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(30 * ELA)},
		{AssetID: DefaultChain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(70 * ELA)},
	}
	err = CheckTransactionBalance(tx)
	assert.EqualError(t, err, "Transaction fee not enough")

	// rollback deposit above
	DefaultChain.(*ChainStore).NewBatch()
	DefaultChain.(*ChainStore).RollbackTransaction(deposit)
	DefaultChain.(*ChainStore).BatchCommit()

	t.Log("[TestCheckTransactionBalance] PASSED")
}

func TestTxValidatorDone(t *testing.T) {
	DefaultChain.Close()
}
