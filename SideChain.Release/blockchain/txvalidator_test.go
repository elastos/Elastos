package blockchain

import (
	"bytes"
	"crypto/rand"
	"fmt"
	"math"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	"github.com/elastos/Elastos.ELA.SideChain/log"

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
	tx := NewCoinBaseTransaction(new(core.PayloadCoinBase), 0)
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
	tx.Inputs = append(tx.Inputs, &core.Input{})
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
	tx.Inputs = append(tx.Inputs, &core.Input{Previous: *core.NewOutPoint(common.EmptyHash, math.MaxUint16)})
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
	tx := NewCoinBaseTransaction(new(core.PayloadCoinBase), 0)
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
	}
	err := CheckTransactionOutput(tx)
	assert.NoError(t, err)

	// outputs < 2
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress},
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "coinbase output is not enough, at least 2")

	// invalid asset id
	tx.Outputs = []*core.Output{
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
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
	}
	err = CheckTransactionOutput(tx)
	assert.NoError(t, err)

	// reward to foundation in coinbase < 30%
	foundationReward = common.Fixed64(float64(totalReward) * 0.2999999)
	t.Logf("Foundation reward amount %s", foundationReward.String())
	minerReward = totalReward - foundationReward
	t.Logf("Miner reward amount %s", minerReward.String())
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: foundationReward},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: minerReward},
	}
	err = CheckTransactionOutput(tx)
	assert.EqualError(t, err, "Reward to foundation in coinbase < 30%")

	// normal transaction
	tx = buildTx()
	for _, output := range tx.Outputs {
		output.AssetID = DefaultLedger.Blockchain.AssetID
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
		output.AssetID = DefaultLedger.Blockchain.AssetID
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
	asset := core.Asset{
		Name:      "TEST",
		Precision: 0x04,
		AssetType: 0x00,
	}
	register := &core.Transaction{
		TxType:         core.RegisterAsset,
		PayloadVersion: 0,
		Payload: &core.PayloadRegisterAsset{
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
	// Fixme method not implemented
	t.Log("[TestCheckAttributeProgram] PASSED")
}

func TestCheckTransactionPayload(t *testing.T) {
	// normal
	tx := new(core.Transaction)
	payload := &core.PayloadRegisterAsset{
		Asset: core.Asset{
			Name:      "ELA",
			Precision: 0x08,
			AssetType: core.Token,
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
	tx := new(core.Transaction)
	tx.TxType = core.WithdrawFromSideChain
	err := CheckTransactionBalance(tx)
	assert.NoError(t, err)

	// deposit 100 ELA to foundation account
	deposit := NewCoinBaseTransaction(new(core.PayloadCoinBase), 0)
	deposit.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(100 * ELA)},
	}
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).PersistTransaction(deposit, 0)
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	// invalid output value
	tx = NewCoinBaseTransaction(new(core.PayloadCoinBase), 0)
	tx.Inputs = []*core.Input{
		{Previous: *core.NewOutPoint(deposit.Hash(), 0)},
	}
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(-20 * ELA)},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(-60 * ELA)},
	}
	err = CheckTransactionBalance(tx)
	assert.EqualError(t, err, "Invalide transaction UTXO output.")

	// invalid transaction fee
	config.Parameters.PowConfiguration.MinTxFee = int(1 * ELA)
	tx.Outputs = []*core.Output{
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: FoundationAddress, Value: common.Fixed64(30 * ELA)},
		{AssetID: DefaultLedger.Blockchain.AssetID, ProgramHash: common.Uint168{}, Value: common.Fixed64(70 * ELA)},
	}
	err = CheckTransactionBalance(tx)
	assert.EqualError(t, err, "Transaction fee not enough")

	// rollback deposit above
	DefaultLedger.Store.(*ChainStore).NewBatch()
	DefaultLedger.Store.(*ChainStore).RollbackTransaction(deposit)
	DefaultLedger.Store.(*ChainStore).BatchCommit()

	t.Log("[TestCheckTransactionBalance] PASSED")
}

func TestTxValidatorDone(t *testing.T) {
	DefaultLedger.Store.Close()
}
