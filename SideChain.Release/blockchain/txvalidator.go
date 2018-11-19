package blockchain

import (
	"bytes"
	"errors"
	"fmt"
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

var TransactionValidator *TransactionValidateBase

type TransactionValidateBase struct {
	CheckTransactionSanity                  func(txn *core.Transaction) ErrCode
	CheckTransactionContext                 func(txn *core.Transaction) ErrCode
	CheckTransactionInput                   func(txn *core.Transaction) error
	CheckTransactionOutput                  func(txn *core.Transaction) error
	CheckOutputProgramHash                  func(programHash Uint168) bool
	CheckTransactionUTXOLock                func(txn *core.Transaction) error
	CheckTransactionSize                    func(txn *core.Transaction) error
	CheckAssetPrecision                     func(txn *core.Transaction) error
	CheckTransactionBalance                 func(txn *core.Transaction) error
	CheckAttributeProgram                   func(tx *core.Transaction) error
	CheckTransactionSignature               func(txn *core.Transaction) error
	CheckAmountPrecise                      func(amount Fixed64, precision byte, assetPrecision byte) bool
	CheckTransactionPayload                 func(txn *core.Transaction) error
	CheckRechargeToSideChainTransaction     func(txn *core.Transaction) error
	CheckTransferCrossChainAssetTransaction func(txn *core.Transaction) error
	CheckTxHashDuplicate                    func(txn *core.Transaction) (bool, ErrCode)
	CheckCoinBaseTx                         func(txn *core.Transaction) (bool, ErrCode)
	CheckSignature                          func(txn *core.Transaction) (bool, ErrCode)
	CheckRechargeToSideChainTx              func(txn *core.Transaction) (bool, ErrCode)
	CheckTransferCrossChainAssetTx          func(txn *core.Transaction) (bool, ErrCode)
	CheckDoubleSpend                        func(txn *core.Transaction) (bool, ErrCode)
	CheckUTXOLock                           func(txn *core.Transaction) (bool, ErrCode)
	CheckBalance                            func(txn *core.Transaction) (bool, ErrCode)
	CheckReferencedOutput                   func(txn *core.Transaction) (bool, ErrCode)
}

func InitTransactionValidtor() {
	TransactionValidator = &TransactionValidateBase{}
	TransactionValidator.Init()
}

func (v *TransactionValidateBase) Init() {
	v.CheckTransactionSanity = v.CheckTransactionSanityImpl
	v.CheckTransactionContext = v.CheckTransactionContextImpl
	v.CheckTransactionInput = v.CheckTransactionInputImpl
	v.CheckTransactionOutput = v.CheckTransactionOutputImpl
	v.CheckOutputProgramHash = v.CheckOutputProgramHashImpl
	v.CheckTransactionUTXOLock = v.CheckTransactionUTXOLockImpl
	v.CheckTransactionSize = v.CheckTransactionSizeImpl
	v.CheckAssetPrecision = v.CheckAssetPrecisionImpl
	v.CheckTransactionBalance = v.CheckTransactionBalanceImpl
	v.CheckAttributeProgram = v.CheckAttributeProgramImpl
	v.CheckTransactionSignature = v.CheckTransactionSignatureImpl
	v.CheckAmountPrecise = v.CheckAmountPreciseImpl
	v.CheckTransactionPayload = v.CheckTransactionPayloadImpl
	v.CheckRechargeToSideChainTransaction = v.CheckRechargeToSideChainTransactionImpl
	v.CheckTransferCrossChainAssetTransaction = v.CheckTransferCrossChainAssetTransactionImpl
	v.CheckTxHashDuplicate = v.CheckTxHashDuplicateImpl
	v.CheckCoinBaseTx = v.CheckCoinBaseTxImpl
	v.CheckSignature = v.CheckSignatureImpl
	v.CheckRechargeToSideChainTx = v.CheckRechargeToSideChainTxImpl
	v.CheckTransferCrossChainAssetTx = v.CheckTransferCrossChainAssetTxImpl
	v.CheckDoubleSpend = v.CheckDoubleSpendImpl
	v.CheckUTXOLock = v.CheckUTXOLockImpl
	v.CheckBalance = v.CheckBalanceImpl
	v.CheckReferencedOutput = v.CheckReferencedOutputImpl

}

// CheckTransactionSanity verifys received single transaction
func (v *TransactionValidateBase) CheckTransactionSanityImpl(txn *core.Transaction) ErrCode {

	if err := v.CheckTransactionSize(txn); err != nil {
		log.Warn("[CheckTransactionSize],", err)
		return ErrTransactionSize
	}

	if err := v.CheckTransactionInput(txn); err != nil {
		log.Warn("[CheckTransactionInput],", err)
		return ErrInvalidInput
	}

	if err := v.CheckTransactionOutput(txn); err != nil {
		log.Warn("[CheckTransactionOutput],", err)
		return ErrInvalidOutput
	}

	if err := v.CheckAssetPrecision(txn); err != nil {
		log.Warn("[CheckAssetPrecesion],", err)
		return ErrAssetPrecision
	}

	if err := v.CheckAttributeProgram(txn); err != nil {
		log.Warn("[CheckAttributeProgram],", err)
		return ErrAttributeProgram
	}

	if err := v.CheckTransactionPayload(txn); err != nil {
		log.Warn("[CheckTransactionPayload],", err)
		return ErrTransactionPayload
	}

	// check iterms above for Coinbase transaction
	if txn.IsCoinBaseTx() {
		return Success
	}

	return Success
}

// CheckTransactionContext verifys a transaction with history transaction in ledger
func (v *TransactionValidateBase) CheckTransactionContextImpl(txn *core.Transaction) ErrCode {
	if ok, errcode := v.CheckTxHashDuplicate(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckCoinBaseTx(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckSignature(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckRechargeToSideChainTx(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckTransferCrossChainAssetTx(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckDoubleSpend(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckUTXOLock(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckBalance(txn); !ok {
		return errcode
	}
	if ok, errcode := v.CheckReferencedOutput(txn); !ok {
		return errcode
	}

	return Success
}

func (v *TransactionValidateBase) CheckTxHashDuplicateImpl(txn *core.Transaction) (bool, ErrCode) {
	// check if duplicated with transaction in ledger
	if exist := DefaultLedger.Store.IsTxHashDuplicate(txn.Hash()); exist {
		log.Info("[CheckTransactionContext] duplicate transaction check faild.")
		return false, ErrTxHashDuplicate
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckCoinBaseTxImpl(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsCoinBaseTx() {
		return false, Success
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckSignatureImpl(txn *core.Transaction) (bool, ErrCode) {
	if err := v.CheckTransactionSignature(txn); err != nil {
		log.Warn("[CheckTransactionSignature],", err)
		return false, ErrTransactionSignature
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckRechargeToSideChainTxImpl(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsRechargeToSideChainTx() {
		if err := v.CheckRechargeToSideChainTransaction(txn); err != nil {
			log.Warn("[CheckRechargeToSideChainTransaction],", err)
			return false, ErrRechargeToSideChain
		}
		return false, Success
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckTransferCrossChainAssetTxImpl(txn *core.Transaction) (bool, ErrCode) {
	if txn.IsTransferCrossChainAssetTx() {
		if err := v.CheckTransferCrossChainAssetTransaction(txn); err != nil {
			log.Warn("[CheckTransferCrossChainAssetTransaction],", err)
			return false, ErrInvalidOutput
		}
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckDoubleSpendImpl(txn *core.Transaction) (bool, ErrCode) {
	// check double spent transaction
	if DefaultLedger.IsDoubleSpend(txn) {
		log.Info("[CheckTransactionContext] IsDoubleSpend check faild.")
		return false, ErrDoubleSpend
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckUTXOLockImpl(txn *core.Transaction) (bool, ErrCode) {
	if err := v.CheckTransactionUTXOLock(txn); err != nil {
		log.Warn("[CheckTransactionUTXOLock],", err)
		return false, ErrUTXOLocked
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckBalanceImpl(txn *core.Transaction) (bool, ErrCode) {
	if err := v.CheckTransactionBalance(txn); err != nil {
		log.Warn("[CheckTransactionBalance],", err)
		return false, ErrTransactionBalance
	}
	return true, Success
}

func (v *TransactionValidateBase) CheckReferencedOutputImpl(txn *core.Transaction) (bool, ErrCode) {
	// check referenced Output value
	for _, input := range txn.Inputs {
		referHash := input.Previous.TxID
		referTxnOutIndex := input.Previous.Index
		referTxn, _, err := DefaultLedger.Store.GetTransaction(referHash)
		if err != nil {
			log.Warn("Referenced transaction can not be found", BytesToHexString(referHash.Bytes()))
			return false, ErrUnknownReferedTxn
		}
		referTxnOut := referTxn.Outputs[referTxnOutIndex]
		if referTxnOut.Value < 0 {
			log.Warn("Value of referenced transaction output is invalid")
			return false, ErrInvalidReferedTxn
		}
		// coinbase transaction only can be spent after got SpendCoinbaseSpan times confirmations
		if referTxn.IsCoinBaseTx() {
			lockHeight := referTxn.LockTime
			currentHeight := DefaultLedger.Store.GetHeight()
			if currentHeight-lockHeight < config.Parameters.ChainParam.SpendCoinbaseSpan {
				return false, ErrIneffectiveCoinbase
			}
		}
	}
	return true, Success
}

//validate the transaction of duplicate UTXO input
func (v *TransactionValidateBase) CheckTransactionInputImpl(txn *core.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Inputs) != 1 {
			return errors.New("coinbase must has only one input")
		}
		coinbaseInputHash := txn.Inputs[0].Previous.TxID
		coinbaseInputIndex := txn.Inputs[0].Previous.Index
		//TODO :check sequence
		if !coinbaseInputHash.IsEqual(EmptyHash) || coinbaseInputIndex != math.MaxUint16 {
			return errors.New("invalid coinbase input")
		}

		return nil
	}

	if txn.IsRechargeToSideChainTx() {
		return nil
	}

	if len(txn.Inputs) <= 0 {
		return errors.New("transaction has no inputs")
	}
	for i, utxoin := range txn.Inputs {
		if utxoin.Previous.TxID.IsEqual(EmptyHash) && (utxoin.Previous.Index == math.MaxUint16) {
			return errors.New("invalid transaction input")
		}
		for j := 0; j < i; j++ {
			if utxoin.Previous.IsEqual(txn.Inputs[j].Previous) {
				return errors.New("duplicated transaction inputs")
			}
		}
	}

	return nil
}

func (v *TransactionValidateBase) CheckTransactionOutputImpl(txn *core.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("coinbase output is not enough, at least 2")
		}

		var totalReward = Fixed64(0)
		var foundationReward = Fixed64(0)
		for _, output := range txn.Outputs {
			if output.AssetID != DefaultLedger.Blockchain.AssetID {
				return errors.New("asset ID in coinbase is invalid")
			}
			totalReward += output.Value
			if output.ProgramHash.IsEqual(FoundationAddress) {
				foundationReward += output.Value
			}
		}
		if Fixed64(foundationReward) < Fixed64(float64(totalReward)*0.3) {
			return errors.New("Reward to foundation in coinbase < 30%")
		}

		return nil
	}

	if len(txn.Outputs) < 1 {
		return errors.New("transaction has no outputs")
	}

	// check if output address is valid
	for _, output := range txn.Outputs {
		if output.AssetID != DefaultLedger.Blockchain.AssetID {
			return errors.New("asset ID in output is invalid")
		}

		if !v.CheckOutputProgramHash(output.ProgramHash) {
			return errors.New("output address is invalid")
		}
	}

	return nil
}

func (v *TransactionValidateBase) CheckOutputProgramHashImpl(programHash Uint168) bool {
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

func (v *TransactionValidateBase) CheckTransactionUTXOLockImpl(txn *core.Transaction) error {
	if txn.IsCoinBaseTx() {
		return nil
	}
	if len(txn.Inputs) <= 0 {
		return errors.New("Transaction has no inputs")
	}
	references, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return fmt.Errorf("GetReference failed: %s", err)
	}
	for input, output := range references {

		if output.OutputLock == 0 {
			//check next utxo
			continue
		}
		if input.Sequence != math.MaxUint32-1 {
			return errors.New("Invalid input sequence")
		}
		if txn.LockTime < output.OutputLock {
			return errors.New("UTXO output locked")
		}
	}
	return nil
}

func (v *TransactionValidateBase) CheckTransactionSizeImpl(txn *core.Transaction) error {
	size := txn.GetSize()
	if size <= 0 || size > config.Parameters.MaxBlockSize {
		return fmt.Errorf("Invalid transaction size: %d bytes", size)
	}

	return nil
}

func (v *TransactionValidateBase) CheckAssetPrecisionImpl(txn *core.Transaction) error {
	if len(txn.Outputs) == 0 {
		return nil
	}
	assetOutputs := make(map[Uint256][]*core.Output, len(txn.Outputs))

	for _, v := range txn.Outputs {
		assetOutputs[v.AssetID] = append(assetOutputs[v.AssetID], v)
	}
	for k, outputs := range assetOutputs {
		asset, err := DefaultLedger.GetAsset(k)
		if err != nil {
			return errors.New("The asset not exist in local blockchain.")
		}
		precision := asset.Precision
		for _, output := range outputs {
			if !v.CheckAmountPrecise(output.Value, precision, core.MaxPrecision) {
				return errors.New("The precision of asset is incorrect.")
			}
		}
	}
	return nil
}

func (v *TransactionValidateBase) CheckTransactionBalanceImpl(txn *core.Transaction) error {
	for _, v := range txn.Outputs {
		if v.Value < Fixed64(0) {
			return errors.New("Invalide transaction UTXO output.")
		}
	}
	results, err := TxFeeHelper.GetTxFeeMap(txn)
	if err != nil {
		return err
	}
	for _, v := range results {
		if v < Fixed64(config.Parameters.PowConfiguration.MinTxFee) {
			return fmt.Errorf("Transaction fee not enough")
		}
	}
	return nil
}

func (v *TransactionValidateBase) CheckAttributeProgramImpl(txn *core.Transaction) error {
	// Check attributes
	for _, attr := range txn.Attributes {
		if !core.IsValidAttributeType(attr.Usage) {
			return fmt.Errorf("invalid attribute usage %v", attr.Usage)
		}
	}

	// Check programs
	for _, program := range txn.Programs {
		if program.Code == nil {
			return fmt.Errorf("invalid program code nil")
		}
		if program.Parameter == nil {
			return fmt.Errorf("invalid program parameter nil")
		}
		_, err := crypto.ToProgramHash(program.Code)
		if err != nil {
			return fmt.Errorf("invalid program code %x", program.Code)
		}
	}
	return nil
}

func (v *TransactionValidateBase) CheckTransactionSignatureImpl(txn *core.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		if err := spv.VerifyTransaction(txn); err != nil {
			return err
		}
		return nil
	}

	hashes, err := GetTxProgramHashes(txn)
	if err != nil {
		return err
	}

	// Sort first
	SortProgramHashes(hashes)
	if err := SortPrograms(txn.Programs); err != nil {
		return err
	}

	return RunPrograms(txn, hashes, txn.Programs)
}

func (v *TransactionValidateBase) CheckAmountPreciseImpl(amount Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (v *TransactionValidateBase) CheckTransactionPayloadImpl(txn *core.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *core.PayloadRegisterAsset:
		if pld.Asset.Precision < core.MinPrecision || pld.Asset.Precision > core.MaxPrecision {
			return errors.New("Invalide asset Precision.")
		}
		if !v.CheckAmountPrecise(pld.Amount, pld.Asset.Precision, core.MaxPrecision) {
			return errors.New("Invalide asset value,out of precise.")
		}
	case *core.PayloadTransferAsset:
	case *core.PayloadRecord:
	case *core.PayloadCoinBase:
	case *core.PayloadRechargeToSideChain:
	case *core.PayloadTransferCrossChainAsset:
	default:
		return errors.New("[txValidator],invalidate transaction payload type.")
	}
	return nil
}

func (v *TransactionValidateBase) CheckRechargeToSideChainTransactionImpl(txn *core.Transaction) error {
	proof := new(MerkleProof)
	mainChainTransaction := new(ela.Transaction)

	payloadRecharge, ok := txn.Payload.(*core.PayloadRechargeToSideChain)
	if !ok {
		return errors.New("Invalid recharge to side chain payload type")
	}

	if config.Parameters.ExchangeRate <= 0 {
		return errors.New("Invalid config exchange rate")
	}

	reader := bytes.NewReader(payloadRecharge.MerkleProof)
	if err := proof.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain payload deserialize failed")
	}
	reader = bytes.NewReader(payloadRecharge.MainChainTransaction)
	if err := mainChainTransaction.Deserialize(reader); err != nil {
		return errors.New("RechargeToSideChain mainChainTransaction deserialize failed")
	}

	mainchainTxhash := mainChainTransaction.Hash()
	if exist := DefaultLedger.Store.IsMainchainTxHashDuplicate(mainchainTxhash); exist {
		return errors.New("Duplicate mainchain transaction hash in paylod")
	}

	payloadObj, ok := mainChainTransaction.Payload.(*ela.PayloadTransferCrossChainAsset)
	if !ok {
		return errors.New("Invalid payload ela.PayloadTransferCrossChainAsset")
	}

	genesisHash, _ := DefaultLedger.Store.GetBlockHash(uint32(0))
	genesisProgramHash, err := common.GetGenesisProgramHash(genesisHash)
	if err != nil {
		return errors.New("Genesis block bytes to program hash failed")
	}

	//check output fee and rate
	var oriOutputTotalAmount Fixed64
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(*genesisProgramHash) {
			if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] >
				mainChainTransaction.Outputs[payloadObj.OutputIndexes[i]].Value-Fixed64(config.Parameters.MinCrossChainTxFee) {
				return errors.New("Invalid transaction cross chain amount")
			}

			crossChainAmount := Fixed64(float64(payloadObj.CrossChainAmounts[i]) * config.Parameters.ExchangeRate)
			oriOutputTotalAmount += crossChainAmount

			programHash, err := Uint168FromAddress(payloadObj.CrossChainAddresses[i])
			if err != nil {
				return errors.New("Invalid transaction payload cross chain address")
			}
			isContained := false
			for _, output := range txn.Outputs {
				if output.ProgramHash == *programHash && output.Value == crossChainAmount {
					isContained = true
					break
				}
			}
			if !isContained {
				return errors.New("Invalid transaction outputs")
			}
		}
	}

	var targetOutputTotalAmount Fixed64
	for _, output := range txn.Outputs {
		if output.Value < 0 {
			return errors.New("Invalid transaction output value")
		}
		targetOutputTotalAmount += output.Value
	}

	if targetOutputTotalAmount != oriOutputTotalAmount {
		return errors.New("Output and fee verify failed")
	}

	return nil
}

func (v *TransactionValidateBase) CheckTransferCrossChainAssetTransactionImpl(txn *core.Transaction) error {
	payloadObj, ok := txn.Payload.(*core.PayloadTransferCrossChainAsset)
	if !ok {
		return errors.New("Invalid transfer cross chain asset payload type")
	}
	if len(payloadObj.CrossChainAddresses) == 0 ||
		len(payloadObj.CrossChainAddresses) > len(txn.Outputs) ||
		len(payloadObj.CrossChainAddresses) != len(payloadObj.CrossChainAmounts) ||
		len(payloadObj.CrossChainAmounts) != len(payloadObj.OutputIndexes) {
		return errors.New("Invalid transaction payload content")
	}

	//check cross chain output index in payload
	outputIndexMap := make(map[uint64]struct{})
	for _, outputIndex := range payloadObj.OutputIndexes {
		if _, exist := outputIndexMap[outputIndex]; exist || int(outputIndex) >= len(txn.Outputs) {
			return errors.New("Invalid transaction payload cross chain index")
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
		return errors.New("Invalid transaction cross chain counts")
	}
	for _, address := range payloadObj.CrossChainAddresses {
		if address == "" {
			return errors.New("Invalid transaction cross chain address")
		}
		programHash, err := Uint168FromAddress(address)
		if err != nil {
			return errors.New("Invalid transaction cross chain address")
		}
		if !bytes.Equal(programHash[0:1], []byte{PrefixStandard}) && !bytes.Equal(programHash[0:1], []byte{PrefixMultisig}) {
			return errors.New("Invalid transaction cross chain address")
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.OutputIndexes); i++ {
		if !txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash.IsEqual(Uint168{}) {
			return errors.New("Invalid transaction output program hash")
		}
		if txn.Outputs[payloadObj.OutputIndexes[i]].Value < 0 || payloadObj.CrossChainAmounts[i] < 0 ||
			payloadObj.CrossChainAmounts[i] > txn.Outputs[payloadObj.OutputIndexes[i]].Value-Fixed64(config.Parameters.MinCrossChainTxFee) {
			return errors.New("Invalid transaction outputs")
		}
	}

	//check transaction fee
	var totalInput Fixed64
	reference, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return errors.New("Invalid transaction inputs")
	}
	for _, v := range reference {
		totalInput += v.Value
	}

	var totalOutput Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}

	if totalInput-totalOutput < Fixed64(config.Parameters.MinCrossChainTxFee) {
		return errors.New("Invalid transaction fee")
	}

	return nil
}
