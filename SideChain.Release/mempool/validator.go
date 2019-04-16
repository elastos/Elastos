package mempool

import (
	"bytes"
	"errors"
	"fmt"
	"math"
	"sort"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	core "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/elanet/bloom"
)

var zeroHash = common.Uint256{}

type TxValidateAction struct {
	Name    FuncName
	Handler func(txn *types.Transaction) error
}

type Validator struct {
	chainParams           *config.Params
	db                    *blockchain.ChainStore
	txFeeHelper           *FeeHelper
	spvService            *spv.Service
	checkSanityFunctions  []*TxValidateAction
	checkContextFunctions []*TxValidateAction
}

func NewValidator(cfg *Config) *Validator {
	v := &Validator{
		chainParams: cfg.ChainParams,
		db:          cfg.ChainStore,
		txFeeHelper: cfg.FeeHelper,
		spvService:  cfg.SpvService,
	}

	v.RegisterSanityFunc(FuncNames.CheckTransactionSize, v.checkTransactionSize)
	v.RegisterSanityFunc(FuncNames.CheckTransactionInput, v.checkTransactionInput)
	v.RegisterSanityFunc(FuncNames.CheckTransactionOutput, v.checkTransactionOutput)
	v.RegisterSanityFunc(FuncNames.CheckAssetPrecision, v.checkAssetPrecision)
	v.RegisterSanityFunc(FuncNames.CheckAttributeProgram, v.checkAttributeProgram)
	v.RegisterSanityFunc(FuncNames.CheckTransactionPayload, v.checkTransactionPayload)

	v.RegisterContextFunc(FuncNames.CheckTransactionDuplicate, v.checkTransactionDuplicate)
	v.RegisterContextFunc(FuncNames.CheckTransactionCoinBase, v.checkTransactionCoinBase)
	v.RegisterContextFunc(FuncNames.CheckTransactionDoubleSpend, v.checkTransactionDoubleSpend)
	v.RegisterContextFunc(FuncNames.CheckTransactionSignature, v.checkTransactionSignature)
	v.RegisterContextFunc(FuncNames.CheckRechargeToSideChainTransaction, v.checkRechargeToSideChainTransaction)
	v.RegisterContextFunc(FuncNames.CheckTransferCrossChainAssetTransaction, v.checkTransferCrossChainAssetTransaction)
	v.RegisterContextFunc(FuncNames.CheckTransactionUTXOLock, v.checkTransactionUTXOLock)
	v.RegisterContextFunc(FuncNames.CheckTransactionBalance, v.checkTransactionBalance)
	v.RegisterContextFunc(FuncNames.CheckReferencedOutput, v.checkReferencedOutput)
	return v
}

func (v *Validator) RegisterSanityFunc(name FuncName, function func(txn *types.Transaction) error) {
	for _, action := range v.checkSanityFunctions {
		if action.Name == name {
			action.Handler = function
			return
		}
	}
	v.checkSanityFunctions = append(v.checkSanityFunctions, &TxValidateAction{Name: name, Handler: function})
}

func (v *Validator) RegisterContextFunc(name FuncName, function func(txn *types.Transaction) error) {
	for _, action := range v.checkContextFunctions {
		if action.Name == name {
			action.Handler = function
			return
		}
	}
	v.checkContextFunctions = append(v.checkContextFunctions, &TxValidateAction{Name: name, Handler: function})
}

// CheckTransactionSanity verifys received single transaction
func (v *Validator) CheckTransactionSanity(txn *types.Transaction) error {
	for _, checkFunc := range v.checkSanityFunctions {
		if err := checkFunc.Handler(txn); err != nil {
			if err == ErrBreak {
				return nil
			}
			return err
		}
	}
	return nil
}

// CheckTransactionContext verifys a transaction with history transaction in ledger
func (v *Validator) CheckTransactionContext(txn *types.Transaction) error {
	for _, checkFunc := range v.checkContextFunctions {
		if err := checkFunc.Handler(txn); err != nil {
			if err == ErrBreak {
				return nil
			}
			return err
		}
	}
	return nil
}

func (v *Validator) checkReferencedOutput(txn *types.Transaction) error {
	// check referenced Output value
	for _, input := range txn.Inputs {
		referHash := input.Previous.TxID
		referTxnOutIndex := input.Previous.Index
		referTxn, _, err := v.db.GetTransaction(referHash)
		if err != nil {
			str := fmt.Sprint("Referenced transaction can not be found ", referHash.String())
			return ruleError(ErrUnknownReferedTx, str)
		}
		referTxnOut := referTxn.Outputs[referTxnOutIndex]
		if referTxnOut.Value < 0 {
			str := fmt.Sprint("Value of referenced transaction output is invalid")
			return ruleError(ErrInvalidReferedTx, str)
		}
		// coinbase transaction only can be spent after got CoinBaseLockTime times confirmations
		if referTxn.IsCoinBaseTx() {
			lockHeight := referTxn.LockTime
			currentHeight := v.db.GetHeight()
			if currentHeight-lockHeight < v.chainParams.CoinbaseMaturity {
				str := fmt.Sprintf("output is locked till %d, current %d", lockHeight, currentHeight)
				return ruleError(ErrIneffectiveCoinbase, str)
			}
		}
	}
	return nil
}

//validate the transaction of duplicate UTXO input
func (v *Validator) checkTransactionInput(txn *types.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Inputs) != 1 {
			str := fmt.Sprint("[checkTransactionInput] coinbase must has only one input")
			return ruleError(ErrInvalidInput, str)
		}
		coinbaseInputHash := txn.Inputs[0].Previous.TxID
		coinbaseInputIndex := txn.Inputs[0].Previous.Index
		//TODO :check sequence
		if !coinbaseInputHash.IsEqual(zeroHash) || coinbaseInputIndex != math.MaxUint16 {
			str := fmt.Sprint("[checkTransactionInput] invalid coinbase input")
			return ruleError(ErrInvalidInput, str)
		}

		return nil
	}

	if txn.IsRechargeToSideChainTx() {
		return nil
	}

	if len(txn.Inputs) <= 0 {
		str := fmt.Sprint("[checkTransactionInput] transaction has no inputs")
		return ruleError(ErrInvalidInput, str)
	}
	for i, utxoin := range txn.Inputs {
		if utxoin.Previous.TxID.IsEqual(zeroHash) && (utxoin.Previous.Index == math.MaxUint16) {
			str := fmt.Sprint("[checkTransactionInput] invalid transaction input")
			return ruleError(ErrInvalidInput, str)
		}
		for j := 0; j < i; j++ {
			if utxoin.Previous.IsEqual(txn.Inputs[j].Previous) {
				str := fmt.Sprint("[checkTransactionInput] duplicated transaction inputs")
				return ruleError(ErrInvalidInput, str)
			}
		}
	}

	return nil
}

func (v *Validator) checkTransactionOutput(txn *types.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			str := fmt.Sprint("[checkTransactionOutput] coinbase output is not enough, at least 2")
			return ruleError(ErrInvalidOutput, str)
		}

		var totalReward = common.Fixed64(0)
		var foundationReward = common.Fixed64(0)
		for _, output := range txn.Outputs {
			if output.AssetID != v.chainParams.ElaAssetId {
				str := fmt.Sprint("[checkTransactionOutput] asset ID in coinbase is invalid")
				return ruleError(ErrInvalidOutput, str)
			}
			totalReward += output.Value
			if output.ProgramHash.IsEqual(v.chainParams.Foundation) {
				foundationReward += output.Value
			}
		}
		if common.Fixed64(foundationReward) < common.Fixed64(float64(totalReward)*0.3) {
			str := fmt.Sprint("[checkTransactionOutput] Reward to foundation in coinbase < 30%")
			return ruleError(ErrInvalidOutput, str)
		}

		return nil
	}

	if len(txn.Outputs) < 1 {
		str := fmt.Sprint("[checkTransactionOutput] transaction has no outputs")
		return ruleError(ErrInvalidOutput, str)
	}

	// check if output address is valid
	for _, output := range txn.Outputs {
		if !output.AssetID.IsEqual(v.chainParams.ElaAssetId) {
			str := fmt.Sprint("[checkTransactionOutput] asset ID in output is invalid")
			return ruleError(ErrInvalidOutput, str)
		}

		if !v.checkOutputProgramHash(output.ProgramHash) {
			str := fmt.Sprint("[checkTransactionOutput] output address is invalid")
			return ruleError(ErrInvalidOutput, str)
		}
	}

	return nil
}

func (v *Validator) checkOutputProgramHash(programHash common.Uint168) bool {
	if programHash.IsEqual(common.Uint168{}) {
		return true
	}

	switch contract.PrefixType(programHash[0]) {
	case contract.PrefixStandard, contract.PrefixMultiSig, contract.PrefixCrossChain:
		return true
	}

	return false
}

func (v *Validator) checkTransactionUTXOLock(txn *types.Transaction) error {
	if txn.IsCoinBaseTx() {
		return nil
	}
	if len(txn.Inputs) <= 0 {
		str := fmt.Sprint("[checkTransactionUTXOLock] Transaction has no inputs")
		return ruleError(ErrUTXOLocked, str)
	}
	references, err := v.db.GetTxReference(txn)
	if err != nil {
		str := fmt.Sprintf("[checkTransactionUTXOLock] GetReference failed: %s", err)
		return ruleError(ErrUTXOLocked, str)
	}
	for input, output := range references {
		if output.OutputLock == 0 {
			//check next utxo
			continue
		}
		if input.Sequence != math.MaxUint32-1 {
			str := fmt.Sprint("[checkTransactionUTXOLock] Invalid input sequence")
			return ruleError(ErrUTXOLocked, str)
		}
		if txn.LockTime < output.OutputLock {
			str := fmt.Sprint("[checkTransactionUTXOLock] UTXO output locked")
			return ruleError(ErrUTXOLocked, str)
		}
	}
	return nil
}

func (v *Validator) checkTransactionSize(txn *types.Transaction) error {
	size := txn.GetSize()
	if size <= 0 || size > types.MaxBlockSize {
		str := fmt.Sprintf("[checkTransactionSize] Invalid transaction size: %d bytes", size)
		return ruleError(ErrTransactionSize, str)
	}

	return nil
}

func (v *Validator) checkAssetPrecision(txn *types.Transaction) error {
	if len(txn.Outputs) == 0 {
		return nil
	}
	assetOutputs := make(map[common.Uint256][]*types.Output, len(txn.Outputs))

	for _, v := range txn.Outputs {
		assetOutputs[v.AssetID] = append(assetOutputs[v.AssetID], v)
	}
	for k, outputs := range assetOutputs {
		asset, err := v.db.GetAsset(k)
		if err != nil {
			str := fmt.Sprint("[checkAssetPrecision] The asset not exist in local blockchain.")
			return ruleError(ErrAssetPrecision, str)
		}
		precision := asset.Precision
		for _, output := range outputs {
			if !v.checkAmountPrecise(output.Value, precision, types.MaxPrecision) {
				str := fmt.Sprint("[checkAssetPrecision] The precision of asset is incorrect.")
				return ruleError(ErrAssetPrecision, str)
			}
		}
	}
	return nil
}

func (v *Validator) checkTransactionBalance(txn *types.Transaction) error {
	for _, v := range txn.Outputs {
		if v.Value < common.Fixed64(0) {
			str := fmt.Sprint("[checkTransactionBalance] Invalide transaction UTXO output.")
			return ruleError(ErrTransactionBalance, str)
		}
	}
	results, err := v.txFeeHelper.GetTxFeeMap(txn)
	if err != nil {
		return ruleError(ErrTransactionBalance, err.Error())
	}
	for _, fee := range results {
		if fee < common.Fixed64(v.chainParams.MinTransactionFee) {
			str := fmt.Sprint("[checkTransactionBalance] Transaction fee not enough")
			return ruleError(ErrTransactionBalance, str)
		}
	}
	return nil
}

func (v *Validator) checkAttributeProgram(txn *types.Transaction) error {
	// Check attributes
	for _, attr := range txn.Attributes {
		if !types.IsValidAttributeType(attr.Usage) {
			str := fmt.Sprintf("[checkAttributeProgram] invalid attribute usage %s", attr.Usage.Name())
			return ruleError(ErrAttributeProgram, str)
		}
	}

	// Check programs
	for _, program := range txn.Programs {
		if program.Code == nil {
			str := fmt.Sprint("[checkAttributeProgram] invalid program code nil")
			return ruleError(ErrAttributeProgram, str)
		}
		if program.Parameter == nil {
			str := fmt.Sprint("[checkAttributeProgram] invalid program parameter nil")
			return ruleError(ErrAttributeProgram, str)
		}
	}
	return nil
}

func (v *Validator) checkTransactionDuplicate(txn *types.Transaction) error {
	// check if duplicated with transaction in ledger
	if exist := v.db.IsDuplicateTx(txn.Hash()); exist {
		str := fmt.Sprint("[CheckTransactionContext] duplicate transaction check faild.")
		return ruleError(ErrTxHashDuplicate, str)
	}
	return nil
}

func (v *Validator) checkTransactionCoinBase(txn *types.Transaction) error {
	if txn.IsCoinBaseTx() {
		return ErrBreak
	}
	return nil
}

func (v *Validator) checkTransactionDoubleSpend(txn *types.Transaction) error {
	// check double spent transaction
	if v.db.IsDoubleSpend(txn) {
		str := fmt.Sprint("[CheckTransactionContext] IsDoubleSpend check faild.")
		return ruleError(ErrDoubleSpend, str)
	}
	return nil
}

func (v *Validator) checkTransactionSignature(tx *types.Transaction) error {
	if tx.IsRechargeToSideChainTx() {
		if err := v.spvService.VerifyTransaction(tx); err != nil {
			return ruleError(ErrTransactionSignature, err.Error())
		}
		return nil
	}

	hashes, err := v.TxProgramHashes(tx)
	if err != nil {
		return ruleError(ErrTransactionSignature, err.Error())
	}

	// Sort first
	common.SortProgramHashByCodeHash(hashes)
	if err := SortPrograms(tx.Programs); err != nil {
		return ruleError(ErrTransactionSignature, err.Error())
	}

	if err := RunPrograms(tx, hashes, tx.Programs); err != nil {
		return ruleError(ErrTransactionSignature, err.Error())
	}

	return nil
}

func (v *Validator) checkAmountPrecise(amount common.Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (v *Validator) checkTransactionPayload(txn *types.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *types.PayloadRegisterAsset:
		if pld.Asset.Precision < types.MinPrecision || pld.Asset.Precision > types.MaxPrecision {
			str := fmt.Sprint("[checkTransactionPayload] Invalide asset Precision.")
			return ruleError(ErrTransactionPayload, str)
		}
		if !v.checkAmountPrecise(pld.Amount, pld.Asset.Precision, types.MaxPrecision) {
			str := fmt.Sprint("[checkTransactionPayload] Invalide asset value,out of precise.")
			return ruleError(ErrTransactionPayload, str)
		}
	case *types.PayloadTransferAsset:
	case *types.PayloadRecord:
	case *types.PayloadCoinBase:
	case *types.PayloadRechargeToSideChain:
	case *types.PayloadTransferCrossChainAsset:
	default:
		str := fmt.Sprint("[checkTransactionPayload] Invalide transaction payload type.")
		return ruleError(ErrTransactionPayload, str)
	}
	return nil
}

func (v *Validator) checkRechargeToSideChainTransaction(txn *types.Transaction) error {
	if !txn.IsRechargeToSideChainTx() {
		return nil
	}

	payloadRecharge, ok := txn.Payload.(*types.PayloadRechargeToSideChain)
	if !ok {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid recharge to side chain payload type")
		return ruleError(ErrRechargeToSideChain, str)
	}

	if v.chainParams.ExchangeRate <= 0 {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid config exchange rate")
		return ruleError(ErrRechargeToSideChain, str)
	}

	mainChainTransaction := new(core.Transaction)
	if txn.PayloadVersion == types.RechargeToSideChainPayloadVersion0 {
		proof := new(bloom.MerkleProof)
		reader := bytes.NewReader(payloadRecharge.MerkleProof)
		if err := proof.Deserialize(reader); err != nil {
			str := fmt.Sprint("[checkRechargeToSideChainTransaction] RechargeToSideChain payload deserialize failed")
			return ruleError(ErrRechargeToSideChain, str)
		}
		reader = bytes.NewReader(payloadRecharge.MainChainTransaction)
		if err := mainChainTransaction.Deserialize(reader); err != nil {
			str := fmt.Sprint("[checkRechargeToSideChainTransaction] RechargeToSideChain mainChainTransaction deserialize failed")
			return ruleError(ErrRechargeToSideChain, str)
		}
	} else if txn.PayloadVersion == types.RechargeToSideChainPayloadVersion1 {
		var err error
		mainChainTransaction, err = v.spvService.GetTransaction(&payloadRecharge.MainChainTransactionHash)
		if err != nil {
			str := fmt.Sprint("[checkRechargeToSideChainTransaction] Get RechargeToSideChain transaction failed")
			return ruleError(ErrRechargeToSideChain, str)
		}
	} else {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid payload version")
		return ruleError(ErrRechargeToSideChain, str)
	}

	mainchainTxhash := mainChainTransaction.Hash()
	if v.db.IsDuplicateMainchainTx(mainchainTxhash) {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Duplicate mainchain transaction hash in paylod")
		return ruleError(ErrRechargeToSideChain, str)
	}

	payloadObj, ok := mainChainTransaction.Payload.(*payload.TransferCrossChainAsset)
	if !ok {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid PayloadTransferCrossChainAsset")
		return ruleError(ErrRechargeToSideChain, str)
	}

	genesisHash, _ := v.db.GetBlockHash(uint32(0))
	genesisProgramHash, err := GenesisToProgramHash(&genesisHash)
	if err != nil {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Genesis block bytes to program hash failed")
		return ruleError(ErrRechargeToSideChain, str)
	}

	//check output fee and rate
	var oriOutputTotalAmount common.Fixed64
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(*genesisProgramHash) {
			if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] >
				mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].Value-
					common.Fixed64(v.chainParams.MinCrossChainTxFee) {
				str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid transaction cross chain amount")
				return ruleError(ErrRechargeToSideChain, str)
			}

			crossChainAmount := common.Fixed64(float64(payloadObj.CrossChainAmounts[i]) *
				v.chainParams.ExchangeRate)
			oriOutputTotalAmount += crossChainAmount

			programHash, err := common.Uint168FromAddress(payloadObj.CrossChainAddresses[i])
			if err != nil {
				str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid transaction payload cross chain address")
				return ruleError(ErrRechargeToSideChain, str)
			}
			isContained := false
			for _, output := range txn.Outputs {
				if output.ProgramHash == *programHash && output.Value == crossChainAmount {
					isContained = true
					break
				}
			}
			if !isContained {
				str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid transaction outputs")
				return ruleError(ErrRechargeToSideChain, str)
			}
		}
	}

	var targetOutputTotalAmount common.Fixed64
	for _, output := range txn.Outputs {
		if output.Value < 0 {
			str := fmt.Sprint("[checkRechargeToSideChainTransaction] Invalid transaction output value")
			return ruleError(ErrRechargeToSideChain, str)
		}
		targetOutputTotalAmount += output.Value
	}

	if targetOutputTotalAmount != oriOutputTotalAmount {
		str := fmt.Sprint("[checkRechargeToSideChainTransaction] Output and fee verify failed")
		return ruleError(ErrRechargeToSideChain, str)
	}

	return ErrBreak
}

func (v *Validator) checkTransferCrossChainAssetTransaction(txn *types.Transaction) error {
	if !txn.IsTransferCrossChainAssetTx() {
		return nil
	}

	payloadObj, ok := txn.Payload.(*types.PayloadTransferCrossChainAsset)
	if !ok {
		str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transfer cross chain asset payload type")
		return ruleError(ErrCrossChain, str)
	}
	if len(payloadObj.CrossChainAddresses) == 0 ||
		len(payloadObj.CrossChainAddresses) > len(txn.Outputs) ||
		len(payloadObj.CrossChainAddresses) != len(payloadObj.CrossChainAmounts) ||
		len(payloadObj.CrossChainAmounts) != len(payloadObj.OutputIndexes) {
		str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction payload content")
		return ruleError(ErrCrossChain, str)
	}

	//check cross chain output index in payload
	outputIndexMap := make(map[uint64]struct{})
	for _, outputIndex := range payloadObj.OutputIndexes {
		if _, exist := outputIndexMap[outputIndex]; exist || int(outputIndex) >= len(txn.Outputs) {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction payload cross chain index")
			return ruleError(ErrCrossChain, str)
		}
		outputIndexMap[outputIndex] = struct{}{}
	}

	//check address in outputs and payload
	var crossChainCount int
	for _, output := range txn.Outputs {
		if output.ProgramHash.IsEqual(common.Uint168{}) {
			crossChainCount++
		}
	}
	if len(payloadObj.CrossChainAddresses) != crossChainCount {
		str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain counts")
		return ruleError(ErrCrossChain, str)
	}
	for _, address := range payloadObj.CrossChainAddresses {
		if address == "" {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return ruleError(ErrCrossChain, str)
		}
		programHash, err := common.Uint168FromAddress(address)
		if err != nil {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return ruleError(ErrCrossChain, str)
		}
		if !bytes.Equal(programHash[0:1], []byte{byte(contract.PrefixStandard)}) && !bytes.Equal(programHash[0:1], []byte{byte(contract.PrefixMultiSig)}) {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return ruleError(ErrCrossChain, str)
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.OutputIndexes); i++ {
		if !txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(common.Uint168{}) {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction output program hash")
			return ruleError(ErrCrossChain, str)
		}
		if txn.Outputs[payloadObj.OutputIndexes[i]].Value < 0 || payloadObj.CrossChainAmounts[i] < 0 ||
			payloadObj.CrossChainAmounts[i] > txn.Outputs[payloadObj.OutputIndexes[i]].Value-
				common.Fixed64(v.chainParams.MinCrossChainTxFee) {
			str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction outputs")
			return ruleError(ErrCrossChain, str)
		}
	}

	//check transaction fee
	var totalInput common.Fixed64
	reference, err := v.db.GetTxReference(txn)
	if err != nil {
		str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction inputs")
		return ruleError(ErrCrossChain, str)
	}
	for _, v := range reference {
		totalInput += v.Value
	}

	var totalOutput common.Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}

	if totalInput-totalOutput < common.Fixed64(v.chainParams.MinCrossChainTxFee) {
		str := fmt.Sprint("[checkTransferCrossChainAssetTransaction] Invalid transaction fee")
		return ruleError(ErrCrossChain, str)
	}

	return nil
}

func GenesisToProgramHash(genesisHash *common.Uint256) (*common.Uint168, error) {
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(genesisHash.Bytes())))
	buf.Write(genesisHash.Bytes())
	buf.WriteByte(byte(common.CROSSCHAIN))

	return common.ToProgramHash(byte(contract.PrefixCrossChain), buf.Bytes()), nil
}

func (v *Validator) TxProgramHashes(tx *types.Transaction) ([]common.Uint168, error) {
	if tx == nil {
		return nil, errors.New("[Transaction],GetProgramHashes transaction is nil.")
	}
	hashes := make([]common.Uint168, 0)
	uniqueHashes := make([]common.Uint168, 0)
	// add inputUTXO's transaction
	references, err := v.db.GetTxReference(tx)
	if err != nil {
		return nil, errors.New("[Transaction], GetProgramHashes failed.")
	}
	for _, output := range references {
		programHash := output.ProgramHash
		hashes = append(hashes, programHash)
	}
	for _, attribute := range tx.Attributes {
		if attribute.Usage == types.Script {
			dataHash, err := common.Uint168FromBytes(attribute.Data)
			if err != nil {
				return nil, errors.New("[Transaction], GetProgramHashes err.")
			}
			hashes = append(hashes, *dataHash)
		}
	}

	//remove duplicated hashes
	uniq := make(map[common.Uint168]bool)
	for _, v := range hashes {
		uniq[v] = true
	}
	for k := range uniq {
		uniqueHashes = append(uniqueHashes, k)
	}
	return uniqueHashes, nil
}

func RunPrograms(tx *types.Transaction, hashes []common.Uint168, programs []*types.Program) error {
	if tx == nil {
		return errors.New("invalid data content nil transaction")
	}
	if len(hashes) != len(programs) {
		return errors.New("number of data hashes is different with number of programs")
	}

	for i := 0; i < len(programs); i++ {
		codeHash := common.ToCodeHash(programs[i].Code)

		if !hashes[i].ToCodeHash().IsEqual(*codeHash) {
			return errors.New("data hash is different from corresponding program code")
		}
		//execute program on VM
		se := vm.NewExecutionEngine(types.GetDataContainer(&hashes[i], tx),
			new(vm.CryptoECDsa), vm.MAXSTEPS, nil, nil)
		se.LoadScript(programs[i].Code, false)
		se.LoadScript(programs[i].Parameter, true)
		se.Execute()

		if se.GetState() != vm.HALT {
			return errors.New("[VM] Finish State not equal to HALT")
		}

		if se.GetEvaluationStack().Count() != 1 {
			return errors.New("[VM] Execute Engine Stack Count Error")
		}

		success := se.GetExecuteResult()
		if !success {
			return errors.New("[VM] Check Sig FALSE")
		}
	}

	return nil
}

func SortPrograms(programs []*types.Program) (err error) {
	sort.Sort(byHash(programs))
	return err
}

type byHash []*types.Program

func (p byHash) Len() int      { return len(p) }
func (p byHash) Swap(i, j int) { p[i], p[j] = p[j], p[i] }
func (p byHash) Less(i, j int) bool {
	hashi := common.ToCodeHash(p[i].Code)
	hashj := common.ToCodeHash(p[j].Code)
	return hashi.Compare(*hashj) < 0
}
