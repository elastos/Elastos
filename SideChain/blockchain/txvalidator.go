package blockchain

import (
	"bytes"
	"math"

	"github.com/elastos/Elastos.ELA.SideChain/common"
	"github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/core"
	. "github.com/elastos/Elastos.ELA.SideChain/errors"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	"github.com/elastos/Elastos.ELA.SideChain/spv"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	. "github.com/elastos/Elastos.ELA/bloom"
	ela "github.com/elastos/Elastos.ELA/core"
)

const (
	CheckTransactionSize    = "checktransactionsize"
	CheckTransactionInput   = "checktransactioninput"
	CheckTransactionOutput  = "checktransactionoutput"
	CheckAssetPrecision     = "checkassetprecision"
	CheckAttributeProgram   = "checkattributeprogram"
	CheckTransactionPayload = "checktransactionpayload"

	CheckTransactionDuplicate               = "checkTransactionDuplicate"
	CheckTransactionCoinBase                = "checktransactioncoinbase"
	CheckTransactionDoubleSpend             = "checktransactiondoublespend"
	CheckTransactionSignature               = "checktransactionsignature"
	CheckRechargeToSideChainTransaction     = "checkrechargetosidechaintransaction"
	CheckTransferCrossChainAssetTransaction = "checktransfercrosschainassettransaction"
	CheckTransactionUTXOLock                = "checktransactionutxolock"
	CheckTransactionBalance                 = "checktransactionbalance"
	CheckReferencedOutput                   = "checkreferencedoutput"
)

var TransactionValidator *TransactionValidate

type TransactionValidateFunctionName string

type TransactionValidate struct {
	CheckSanityFunctions  map[TransactionValidateFunctionName]func(txn *core.Transaction) ErrCode
	CheckContextFunctions map[TransactionValidateFunctionName]func(txn *core.Transaction) (bool, ErrCode)
}

func InitTransactionValidtor() {
	TransactionValidator = &TransactionValidate{}
	TransactionValidator.RegisterSanityFunctions(CheckTransactionSize, TransactionValidator.checkTransactionSize)
	TransactionValidator.RegisterSanityFunctions(CheckTransactionInput, TransactionValidator.checkTransactionInput)
	TransactionValidator.RegisterSanityFunctions(CheckTransactionOutput, TransactionValidator.checkTransactionOutput)
	TransactionValidator.RegisterSanityFunctions(CheckAssetPrecision, TransactionValidator.checkAssetPrecision)
	TransactionValidator.RegisterSanityFunctions(CheckAttributeProgram, TransactionValidator.checkAttributeProgram)
	TransactionValidator.RegisterSanityFunctions(CheckTransactionPayload, TransactionValidator.checkTransactionPayload)

	TransactionValidator.RegisterContextFunctions(CheckTransactionDuplicate, TransactionValidator.checkTransactionDuplicate)
	TransactionValidator.RegisterContextFunctions(CheckTransactionCoinBase, TransactionValidator.checkTransactionCoinBase)
	TransactionValidator.RegisterContextFunctions(CheckTransactionDoubleSpend, TransactionValidator.checkTransactionDoubleSpend)
	TransactionValidator.RegisterContextFunctions(CheckTransactionSignature, TransactionValidator.checkTransactionSignature)
	TransactionValidator.RegisterContextFunctions(CheckRechargeToSideChainTransaction, TransactionValidator.checkRechargeToSideChainTransaction)
	TransactionValidator.RegisterContextFunctions(CheckTransferCrossChainAssetTransaction, TransactionValidator.checkTransferCrossChainAssetTransaction)
	TransactionValidator.RegisterContextFunctions(CheckTransactionUTXOLock, TransactionValidator.checkTransactionUTXOLock)
	TransactionValidator.RegisterContextFunctions(CheckTransactionBalance, TransactionValidator.checkTransactionBalance)
	TransactionValidator.RegisterContextFunctions(CheckReferencedOutput, TransactionValidator.checkReferencedOutput)
}

func (txv *TransactionValidate) RegisterSanityFunctions(funcName TransactionValidateFunctionName, function func(txn *core.Transaction) ErrCode) {
	txv.CheckSanityFunctions[funcName] = function
}

func (txv *TransactionValidate) RegisterContextFunctions(funcName TransactionValidateFunctionName, function func(txn *core.Transaction) (bool, ErrCode)) {
	txv.CheckContextFunctions[funcName] = function
}

// CheckTransactionSanity verifys received single transaction
func (txv *TransactionValidate) CheckTransactionSanity(txn *core.Transaction) ErrCode {
	for _, checkFunc := range txv.CheckSanityFunctions {
		if errcode := checkFunc(txn); errcode != Success {
			return errcode
		}
	}
	return Success
}

// CheckTransactionContext verifys a transaction with history transaction in ledger
func (txv *TransactionValidate) CheckTransactionContext(txn *core.Transaction) ErrCode {
	for _, checkFunc := range txv.CheckContextFunctions {
		if ok, errcode := checkFunc(txn); !ok {
			return errcode
		}
	}

	return Success
}

func (txv *TransactionValidate) checkReferencedOutput(txn *core.Transaction) (bool, ErrCode) {
	// check referenced Output value
	for _, input := range txn.Inputs {
		referHash := input.Previous.TxID
		referTxnOutIndex := input.Previous.Index
		referTxn, _, err := DefaultChain.GetTransaction(referHash)
		if err != nil {
			log.Warn("Referenced transaction can not be found", BytesToHexString(referHash.Bytes()))
			return false, ErrUnknownReferedTxn
		}
		referTxnOut := referTxn.Outputs[referTxnOutIndex]
		if referTxnOut.Value <= 0 {
			log.Warn("Value of referenced transaction output is invalid")
			return false, ErrInvalidReferedTxn
		}
		// coinbase transaction only can be spent after got SpendCoinbaseSpan times confirmations
		if referTxn.IsCoinBaseTx() {
			lockHeight := referTxn.LockTime
			currentHeight := DefaultChain.GetBestHeight()
			if currentHeight-lockHeight < config.Parameters.ChainParam.SpendCoinbaseSpan {
				return false, ErrIneffectiveCoinbase
			}
		}
	}
	return true, Success
}

//validate the transaction of duplicate UTXO input
func (txv *TransactionValidate) checkTransactionInput(txn *core.Transaction) ErrCode {
	if txn.IsCoinBaseTx() {
		if len(txn.Inputs) != 1 {
			log.Warn("[checkTransactionInput] coinbase must has only one input")
			return ErrInvalidInput
		}
		coinbaseInputHash := txn.Inputs[0].Previous.TxID
		coinbaseInputIndex := txn.Inputs[0].Previous.Index
		//TODO :check sequence
		if !coinbaseInputHash.IsEqual(EmptyHash) || coinbaseInputIndex != math.MaxUint16 {
			log.Warn("[checkTransactionInput] invalid coinbase input")
			return ErrInvalidInput
		}

		return Success
	}

	if txn.IsRechargeToSideChainTx() {
		return Success
	}

	if len(txn.Inputs) <= 0 {
		log.Warn("[checkTransactionInput] transaction has no inputs")
		return ErrInvalidInput
	}
	for i, utxoin := range txn.Inputs {
		if utxoin.Previous.TxID.IsEqual(EmptyHash) && (utxoin.Previous.Index == math.MaxUint16) {
			log.Warn("[checkTransactionInput] invalid transaction input")
			return ErrInvalidInput
		}
		for j := 0; j < i; j++ {
			if utxoin.Previous.IsEqual(txn.Inputs[j].Previous) {
				log.Warn("[checkTransactionInput] duplicated transaction inputs")
				return ErrInvalidInput
			}
		}
	}

	return Success
}

func (txv *TransactionValidate) checkTransactionOutput(txn *core.Transaction) ErrCode {
	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			log.Warn("[checkTransactionOutput] coinbase output is not enough, at least 2")
			return ErrInvalidOutput
		}

		var totalReward = Fixed64(0)
		var foundationReward = Fixed64(0)
		for _, output := range txn.Outputs {
			if output.AssetID != DefaultChain.AssetID {
				log.Warn("[checkTransactionOutput] asset ID in coinbase is invalid")
				return ErrInvalidOutput
			}
			totalReward += output.Value
			if output.ProgramHash.IsEqual(FoundationAddress) {
				foundationReward += output.Value
			}
		}
		if Fixed64(foundationReward) < Fixed64(float64(totalReward)*0.3) {
			log.Warn("[checkTransactionOutput] Reward to foundation in coinbase < 30%")
			return ErrInvalidOutput
		}

		return Success
	}

	if len(txn.Outputs) < 1 {
		log.Warn("[checkTransactionOutput] transaction has no outputs")
		return ErrInvalidOutput
	}

	// check if output address is valid
	for _, output := range txn.Outputs {
		if output.AssetID != DefaultChain.AssetID {
			log.Warn("[checkTransactionOutput] asset ID in output is invalid")
			return ErrInvalidOutput
		}

		if !txv.CheckOutputProgramHashImpl(output.ProgramHash) {
			log.Warn("[checkTransactionOutput] output address is invalid")
			return ErrInvalidOutput
		}
	}

	return Success
}

func (txv *TransactionValidate) CheckOutputProgramHashImpl(programHash Uint168) bool {
	var empty = Uint168{}
	prefix := programHash[0]
	if prefix == PrefixStandard ||
		prefix == PrefixMultisig ||
		prefix == PrefixCrossChain ||
		prefix == PrefixRegisterId ||
		programHash == empty {
		return true
	}
	return false
}

func (txv *TransactionValidate) checkTransactionUTXOLock(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsCoinBaseTx() {
		return true, Success
	}
	if len(txn.Inputs) <= 0 {
		log.Warn("[checkTransactionUTXOLock] Transaction has no inputs")
		return false, ErrUTXOLocked
	}
	references, err := DefaultChain.GetTxReference(txn)
	if err != nil {
		log.Warn("[checkTransactionUTXOLock] GetReference failed: %s", err)
		return false, ErrUTXOLocked
	}
	for input, output := range references {

		if output.OutputLock == 0 {
			//check next utxo
			continue
		}
		if input.Sequence != math.MaxUint32-1 {
			log.Warn("[checkTransactionUTXOLock] Invalid input sequence")
			return false, ErrUTXOLocked
		}
		if txn.LockTime < output.OutputLock {
			log.Warn("[checkTransactionUTXOLock] UTXO output locked")
			return false, ErrUTXOLocked
		}
	}
	return true, Success
}

func (txv *TransactionValidate) checkTransactionSize(txn *core.Transaction) ErrCode {
	size := txn.GetSize()
	if size <= 0 || size > config.Parameters.MaxBlockSize {
		log.Warn("[checkTransactionSize] Invalid transaction size: %d bytes", size)
		return ErrTransactionSize
	}

	return Success
}

func (txv *TransactionValidate) checkAssetPrecision(txn *core.Transaction) ErrCode {
	if len(txn.Outputs) == 0 {
		return Success
	}
	assetOutputs := make(map[Uint256][]*core.Output, len(txn.Outputs))

	for _, v := range txn.Outputs {
		assetOutputs[v.AssetID] = append(assetOutputs[v.AssetID], v)
	}
	for k, outputs := range assetOutputs {
		asset, err := DefaultChain.GetAsset(k)
		if err != nil {
			log.Warn("[checkAssetPrecision] The asset not exist in local blockchain.")
			return ErrAssetPrecision
		}
		precision := asset.Precision
		for _, output := range outputs {
			if !txv.CheckAmountPreciseImpl(output.Value, precision, core.MaxPrecision) {
				log.Warn("[checkAssetPrecision] The precision of asset is incorrect.")
				return ErrAssetPrecision
			}
		}
	}
	return Success
}

func (txv *TransactionValidate) checkTransactionBalance(txn *core.Transaction) (bool, ErrCode) {
	for _, v := range txn.Outputs {
		if v.Value < Fixed64(0) {
			log.Warn("[checkTransactionBalance] Invalide transaction UTXO output.")
			return false, ErrTransactionBalance
		}
	}
	results, err := TxFeeHelper.GetTxFeeMap(txn)
	if err != nil {
		return false, ErrTransactionBalance
	}
	for _, v := range results {
		if v < Fixed64(config.Parameters.PowConfiguration.MinTxFee) {
			log.Warn("[checkTransactionBalance] Transaction fee not enough")
			return false, ErrTransactionBalance
		}
	}
	return true, Success
}

func (txv *TransactionValidate) checkAttributeProgram(txn *core.Transaction) ErrCode {
	// Check attributes
	for _, attr := range txn.Attributes {
		if !core.IsValidAttributeType(attr.Usage) {
			log.Warn("[checkAttributeProgram] invalid attribute usage %txv", attr.Usage)
			return ErrAttributeProgram
		}
	}

	// Check programs
	for _, program := range txn.Programs {
		if program.Code == nil {
			log.Warn("[checkAttributeProgram] invalid program code nil")
			return ErrAttributeProgram
		}
		if program.Parameter == nil {
			log.Warn("[checkAttributeProgram] invalid program parameter nil")
			return ErrAttributeProgram
		}
		_, err := crypto.ToProgramHash(program.Code)
		if err != nil {
			log.Warn("[checkAttributeProgram] invalid program code %x", program.Code)
			return ErrAttributeProgram
		}
	}
	return Success
}

func (txv *TransactionValidate) checkTransactionDuplicate(txn *core.Transaction) (bool, ErrCode) {
	// check if duplicated with transaction in ledger
	if exist := DefaultChain.IsDuplicateTx(txn.Hash()); exist {
		log.Info("[CheckTransactionContext] duplicate transaction check faild.")
		return false, ErrTxHashDuplicate
	}
	return true, Success
}

func (txv *TransactionValidate) checkTransactionCoinBase(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsCoinBaseTx() {
		return false, Success
	}
	return true, Success
}

func (txv *TransactionValidate) checkTransactionDoubleSpend(txn *core.Transaction) (bool, ErrCode) {
	// check double spent transaction
	if DefaultChain.IsDoubleSpend(txn) {
		log.Warn("[CheckTransactionContext] IsDoubleSpend check faild.")
		return false, ErrDoubleSpend
	}
	return true, Success
}

func (txv *TransactionValidate) checkTransactionSignature(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsRechargeToSideChainTx() {
		if err := spv.VerifyTransaction(txn); err != nil {
			return false, ErrTransactionSignature
		}
		return true, Success
	}

	hashes, err := GetTxProgramHashes(txn)
	if err != nil {
		return false, ErrTransactionSignature
	}

	// Sort first
	SortProgramHashes(hashes)
	if err := SortPrograms(txn.Programs); err != nil {
		return false, ErrTransactionSignature
	}

	if err := RunPrograms(txn, hashes, txn.Programs); err != nil {
		return false, ErrTransactionSignature
	}

	return true, Success
}

func (txv *TransactionValidate) CheckAmountPreciseImpl(amount Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (txv *TransactionValidate) checkTransactionPayload(txn *core.Transaction) ErrCode {
	switch pld := txn.Payload.(type) {
	case *core.PayloadRegisterAsset:
		if pld.Asset.Precision < core.MinPrecision || pld.Asset.Precision > core.MaxPrecision {
			log.Warn("[checkTransactionPayload] Invalide asset Precision.")
			return ErrTransactionPayload
		}
		if !txv.CheckAmountPreciseImpl(pld.Amount, pld.Asset.Precision, core.MaxPrecision) {
			log.Warn("[checkTransactionPayload] Invalide asset value,out of precise.")
			return ErrTransactionPayload
		}
	case *core.PayloadTransferAsset:
	case *core.PayloadRecord:
	case *core.PayloadCoinBase:
	case *core.PayloadRechargeToSideChain:
	case *core.PayloadTransferCrossChainAsset:
	default:
		log.Warn("[checkTransactionPayload] Invalide transaction payload type.")
		return ErrTransactionPayload
	}
	return Success
}

func (txv *TransactionValidate) checkRechargeToSideChainTransaction(txn *core.Transaction) (bool, ErrCode) {
	if !txn.IsRechargeToSideChainTx() {
		return true, Success
	}

	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	payloadRecharge, ok := txn.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		log.Warn("[checkRechargeToSideChainTransaction] Invalid recharge to side chain payload type")
		return false, ErrRechargeToSideChain
	}

	if config.Parameters.ExchangeRate <= 0 {
		log.Warn("[checkRechargeToSideChainTransaction] Invalid config exchange rate")
		return false, ErrRechargeToSideChain
	}

	reader := bytes.NewReader(payloadRecharge.MerkleProof)
	if err := proof.Deserialize(reader); err != nil {
		log.Warn("[checkRechargeToSideChainTransaction] RechargeToSideChain payload deserialize failed")
		return false, ErrRechargeToSideChain
	}
	reader = bytes.NewReader(payloadRecharge.MainChainTransaction)
	if err := mainChainTransaction.Deserialize(reader); err != nil {
		log.Warn("[checkRechargeToSideChainTransaction] RechargeToSideChain mainChainTransaction deserialize failed")
		return false, ErrRechargeToSideChain
	}

	mainchainTxhash := mainChainTransaction.Hash()
	if DefaultChain.IsDuplicateMainchainTx(mainchainTxhash) {
		log.Warn("[checkRechargeToSideChainTransaction] Duplicate mainchain transaction hash in paylod")
		return false, ErrRechargeToSideChain
	}

	payloadObj, ok := mainChainTransaction.Payload.(*ela.PayloadTransferCrossChainAsset)
	if !ok {
		log.Warn("[checkRechargeToSideChainTransaction] Invalid payload ela.PayloadTransferCrossChainAsset")
		return false, ErrRechargeToSideChain
	}

	genesisHash, _ := DefaultChain.GetBlockHash(uint32(0))
	genesisProgramHash, err := common.GetGenesisProgramHash(genesisHash)
	if err != nil {
		log.Warn("[checkRechargeToSideChainTransaction] Genesis block bytes to program hash failed")
		return false, ErrRechargeToSideChain
	}

	//check output fee and rate
	var oriOutputTotalAmount Fixed64
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(*genesisProgramHash) {
			if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] >
				mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].Value-Fixed64(config.Parameters.MinCrossChainTxFee) {
				log.Warn("[checkRechargeToSideChainTransaction] Invalid transaction cross chain amount")
				return false, ErrRechargeToSideChain
			}

			crossChainAmount := Fixed64(float64(payloadObj.CrossChainAmounts[i]) * config.Parameters.ExchangeRate)
			oriOutputTotalAmount += crossChainAmount

			programHash, err := Uint168FromAddress(payloadObj.CrossChainAddresses[i])
			if err != nil {
				log.Warn("[checkRechargeToSideChainTransaction] Invalid transaction payload cross chain address")
				return false, ErrRechargeToSideChain
			}
			isContained := false
			for _, output := range txn.Outputs {
				if output.ProgramHash == *programHash && output.Value == crossChainAmount {
					isContained = true
					break
				}
			}
			if !isContained {
				log.Warn("[checkRechargeToSideChainTransaction] Invalid transaction outputs")
				return false, ErrRechargeToSideChain
			}
		}
	}

	var targetOutputTotalAmount Fixed64
	for _, output := range txn.Outputs {
		if output.Value < 0 {
			log.Warn("[checkRechargeToSideChainTransaction] Invalid transaction output value")
			return false, ErrRechargeToSideChain
		}
		targetOutputTotalAmount += output.Value
	}

	if targetOutputTotalAmount != oriOutputTotalAmount {
		log.Warn("[checkRechargeToSideChainTransaction] Output and fee verify failed")
		return false, ErrRechargeToSideChain
	}

	return false, Success
}

func (txv *TransactionValidate) checkTransferCrossChainAssetTransaction(txn *core.Transaction) (bool, ErrCode) {
	if !txn.IsTransferCrossChainAssetTx() {
		return true, Success
	}

	payloadObj, ok := txn.Payload.(*core.PayloadTransferCrossChainAsset)
	if !ok {
		log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transfer cross chain asset payload type")
		return false, ErrCrossChain
	}
	if len(payloadObj.CrossChainAddresses) == 0 ||
		len(payloadObj.CrossChainAddresses) > len(txn.Outputs) ||
		len(payloadObj.CrossChainAddresses) != len(payloadObj.CrossChainAmounts) ||
		len(payloadObj.CrossChainAmounts) != len(payloadObj.OutputIndexes) {
		log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction payload content")
		return false, ErrCrossChain
	}

	//check cross chain output index in payload
	outputIndexMap := make(map[uint64]struct{})
	for _, outputIndex := range payloadObj.OutputIndexes {
		if _, exist := outputIndexMap[outputIndex]; exist || int(outputIndex) >= len(txn.Outputs) {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction payload cross chain index")
			return false, ErrCrossChain
		}
		outputIndexMap[outputIndex] = struct{}{}
	}

	//check address in outputs and payload
	var crossChainCount int
	for _, output := range txn.Outputs {
		if output.ProgramHash.IsEqual(Uint168{}) {
			crossChainCount++
		}
	}
	if len(payloadObj.CrossChainAddresses) != crossChainCount {
		log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain counts")
		return false, ErrCrossChain
	}
	for _, address := range payloadObj.CrossChainAddresses {
		if address == "" {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return false, ErrCrossChain
		}
		programHash, err := Uint168FromAddress(address)
		if err != nil {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return false, ErrCrossChain
		}
		if !bytes.Equal(programHash[0:1], []byte{PrefixStandard}) && !bytes.Equal(programHash[0:1], []byte{PrefixMultisig}) {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction cross chain address")
			return false, ErrCrossChain
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.OutputIndexes); i++ {
		if !txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(Uint168{}) {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction output program hash")
			return false, ErrCrossChain
		}
		if txn.Outputs[payloadObj.OutputIndexes[i]].Value < 0 || payloadObj.CrossChainAmounts[i] < 0 ||
			payloadObj.CrossChainAmounts[i] > txn.Outputs[payloadObj.OutputIndexes[i]].Value-Fixed64(config.Parameters.MinCrossChainTxFee) {
			log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction outputs")
			return false, ErrCrossChain
		}
	}

	//check transaction fee
	var totalInput Fixed64
	reference, err := DefaultChain.GetTxReference(txn)
	if err != nil {
		log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction inputs")
		return false, ErrCrossChain
	}
	for _, v := range reference {
		totalInput += v.Value
	}

	var totalOutput Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}

	if totalInput-totalOutput < Fixed64(config.Parameters.MinCrossChainTxFee) {
		log.Warn("[checkTransferCrossChainAssetTransaction] Invalid transaction fee")
		return false, ErrCrossChain
	}

	return true, Success
}
