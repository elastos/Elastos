package blockchain

import (
	"bytes"
	"errors"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/log"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA.Utility/crypto"
)

// CheckTransactionSanity verifys received single transaction
func CheckTransactionSanity(version uint32, txn *Transaction) ErrCode {
	if err := CheckTransactionSize(txn); err != nil {
		log.Warn("[CheckTransactionSize],", err)
		return ErrTransactionSize
	}

	if err := CheckTransactionInput(txn); err != nil {
		log.Warn("[CheckTransactionInput],", err)
		return ErrInvalidInput
	}

	if err := CheckTransactionOutput(version, txn); err != nil {
		log.Warn("[CheckTransactionOutput],", err)
		return ErrInvalidOutput
	}

	if err := CheckAssetPrecision(txn); err != nil {
		log.Warn("[CheckAssetPrecesion],", err)
		return ErrAssetPrecision
	}

	if err := CheckAttributeProgram(txn); err != nil {
		log.Warn("[CheckAttributeProgram],", err)
		return ErrAttributeProgram
	}

	if err := CheckTransactionPayload(txn); err != nil {
		log.Warn("[CheckTransactionPayload],", err)
		return ErrTransactionPayload
	}

	if err := CheckDuplicateSidechainTx(txn); err != nil {
		log.Warn("[CheckDuplicateSidechainTx],", err)
		return ErrSidechainTxDuplicate
	}

	// check iterms above for Coinbase transaction
	if txn.IsCoinBaseTx() {
		return Success
	}

	return Success
}

// CheckTransactionContext verifys a transaction with history transaction in ledger
func CheckTransactionContext(txn *Transaction) ErrCode {
	// check if duplicated with transaction in ledger
	if exist := DefaultLedger.Store.IsTxHashDuplicate(txn.Hash()); exist {
		log.Warn("[CheckTransactionContext] duplicate transaction check faild.")
		return ErrTransactionDuplicate
	}

	if txn.IsCoinBaseTx() {
		return Success
	}

	if txn.IsSideChainPowTx() {
		arbitrtor, err := GetCurrentArbiter()
		if err != nil {
			return ErrSideChainPowConsensus
		}
		if err = CheckSideChainPowConsensus(txn, arbitrtor); err != nil {
			log.Warn("[CheckSideChainPowConsensus],", err)
			return ErrSideChainPowConsensus
		}
	}

	if txn.IsWithdrawFromSideChainTx() {
		if err := CheckWithdrawFromSideChainTransaction(txn); err != nil {
			log.Warn("[CheckWithdrawFromSideChainTransaction],", err)
			return ErrSidechainTxDuplicate
		}
	}

	if txn.IsTransferCrossChainAssetTx() {
		if err := CheckTransferCrossChainAssetTransaction(txn); err != nil {
			log.Warn("[CheckTransferCrossChainAssetTransaction],", err)
			return ErrInvalidOutput
		}
	}

	// check double spent transaction
	if DefaultLedger.IsDoubleSpend(txn) {
		log.Warn("[CheckTransactionContext] IsDoubleSpend check faild.")
		return ErrDoubleSpend
	}

	if err := CheckTransactionUTXOLock(txn); err != nil {
		log.Warn("[CheckTransactionUTXOLock],", err)
		return ErrUTXOLocked
	}

	if err := CheckTransactionBalance(txn); err != nil {
		log.Warn("[CheckTransactionBalance],", err)
		return ErrTransactionBalance
	}

	if err := CheckTransactionSignature(txn); err != nil {
		log.Warn("[CheckTransactionSignature],", err)
		return ErrTransactionSignature
	}
	// check referenced Output value
	for _, input := range txn.Inputs {
		referHash := input.Previous.TxID
		referTxnOutIndex := input.Previous.Index
		referTxn, _, err := DefaultLedger.Store.GetTransaction(referHash)
		if err != nil {
			log.Warn("Referenced transaction can not be found", BytesToHexString(referHash.Bytes()))
			return ErrUnknownReferedTx
		}
		referTxnOut := referTxn.Outputs[referTxnOutIndex]
		if referTxnOut.Value < 0 {
			log.Warn("Value of referenced transaction output is invalid")
			return ErrInvalidReferedTx
		}
		// coinbase transaction only can be spent after got CoinbaseLockTime times confirmations
		if referTxn.IsCoinBaseTx() {
			lockHeight := referTxn.LockTime
			currentHeight := DefaultLedger.Store.GetHeight()
			if currentHeight-lockHeight < config.Parameters.ChainParam.CoinbaseLockTime {
				return ErrIneffectiveCoinbase
			}
		}
	}

	return Success
}

//validate the transaction of duplicate UTXO input
func CheckTransactionInput(txn *Transaction) error {
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

func CheckTransactionOutput(version uint32, txn *Transaction) error {
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
	if version&CheckTxOut == CheckTxOut {
		for _, output := range txn.Outputs {
			if output.AssetID != DefaultLedger.Blockchain.AssetID {
				return errors.New("asset ID in output is invalid")
			}

			if !CheckOutputProgramHash(output.ProgramHash) {
				return errors.New("output address is invalid")
			}
		}
	}

	return nil
}

func CheckOutputProgramHash(programHash Uint168) bool {
	var empty = Uint168{}
	prefix := programHash[0]
	if prefix == PrefixStandard ||
		prefix == PrefixMultisig ||
		prefix == PrefixCrossChain ||
		programHash == empty {
		return true
	}
	return false
}

func CheckTransactionUTXOLock(txn *Transaction) error {
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

func CheckTransactionSize(txn *Transaction) error {
	size := txn.GetSize()
	if size <= 0 || size > config.Parameters.MaxBlockSize {
		return fmt.Errorf("Invalid transaction size: %d bytes", size)
	}

	return nil
}

func CheckAssetPrecision(txn *Transaction) error {
	if len(txn.Outputs) == 0 {
		return nil
	}
	assetOutputs := make(map[Uint256][]*Output)

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
			if !checkAmountPrecise(output.Value, precision) {
				return errors.New("The precision of asset is incorrect.")
			}
		}
	}
	return nil
}

func CheckTransactionBalance(tx *Transaction) error {
	// output value must >= 0
	for _, output := range tx.Outputs {
		if output.Value < Fixed64(0) {
			return errors.New("Invalide transaction UTXO output.")
		}
	}

	results, err := GetTxFeeMap(tx)
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

func CheckAttributeProgram(tx *Transaction) error {
	// Coinbase transaction do not check attribute and program
	if tx.IsCoinBaseTx() {
		return nil
	}

	// Check attributes
	for _, attr := range tx.Attributes {
		if !IsValidAttributeType(attr.Usage) {
			return fmt.Errorf("invalid attribute usage %v", attr.Usage)
		}
	}

	// Check programs
	if len(tx.Programs) == 0 {
		return fmt.Errorf("no programs found in transaction")
	}
	for _, program := range tx.Programs {
		if program.Code == nil {
			return fmt.Errorf("invalid program code nil")
		}
		if program.Parameter == nil {
			return fmt.Errorf("invalid program parameter nil")
		}
		_, err := ToProgramHash(program.Code)
		if err != nil {
			return fmt.Errorf("invalid program code %x", program.Code)
		}
	}
	return nil
}

func CheckTransactionSignature(txn *Transaction) error {
	return VerifySignature(txn)
}

func checkAmountPrecise(amount Fixed64, precision byte) bool {
	return amount.IntValue()%int64(math.Pow(10, float64(8-precision))) == 0
}

func CheckTransactionPayload(txn *Transaction) error {
	switch pld := txn.Payload.(type) {
	case *PayloadRegisterAsset:
		if pld.Asset.Precision < MinPrecision || pld.Asset.Precision > MaxPrecision {
			return errors.New("Invalide asset Precision.")
		}
		if !checkAmountPrecise(pld.Amount, pld.Asset.Precision) {
			return errors.New("Invalide asset value,out of precise.")
		}
	case *PayloadTransferAsset:
	case *PayloadRecord:
	case *PayloadCoinBase:
	case *PayloadSideChainPow:
	case *PayloadWithdrawFromSideChain:
	case *PayloadTransferCrossChainAsset:
	default:
		return errors.New("[txValidator],invalidate transaction payload type.")
	}
	return nil
}

//validate the transaction of duplicate sidechain transaction
func CheckDuplicateSidechainTx(txn *Transaction) error {
	if txn.IsWithdrawFromSideChainTx() {
		witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
		existingHashs := make(map[Uint256]struct{})
		for _, hash := range witPayload.SideChainTransactionHashes {
			if _, exist := existingHashs[hash]; exist {
				return errors.New("Duplicate sidechain tx detected in a transaction")
			}
			existingHashs[hash] = struct{}{}
		}
	}
	return nil
}

func GetCurrentArbiter() ([]byte, error) {
	arbitrators, err := config.Parameters.GetArbitrators()
	if err != nil {
		return nil, err
	}
	height := DefaultLedger.Store.GetHeight()
	index := height % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator, nil
}

func CheckSideChainPowConsensus(txn *Transaction, arbitrator []byte) error {
	payloadSideChainPow, ok := txn.Payload.(*PayloadSideChainPow)
	if !ok {
		return errors.New("Side mining transaction has invalid payload")
	}

	publicKey, err := DecodePoint(arbitrator)
	if err != nil {
		return err
	}

	buf := new(bytes.Buffer)
	err = payloadSideChainPow.Serialize(buf, SideChainPowPayloadVersion)
	if err != nil {
		return err
	}

	err = Verify(*publicKey, buf.Bytes()[0:68], payloadSideChainPow.SignedData)
	if err != nil {
		return errors.New("Arbitrator is not matched")
	}

	return nil
}

func CheckWithdrawFromSideChainTransaction(txn *Transaction) error {
	witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
	for _, hash := range witPayload.SideChainTransactionHashes {
		if exist := DefaultLedger.Store.IsSidechainTxHashDuplicate(hash); exist {
			return errors.New("Duplicate side chain transaction hash in paylod")
		}
	}

	reference, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		return errors.New("Invalid transaction inputs")
	}
	for _, v := range reference {
		if bytes.Compare(v.ProgramHash[0:1], []byte{PrefixCrossChain}) != 0 {
			return errors.New("Invalid transaction inputs address, without \"X\" at beginning")
		}
	}

	return nil
}

func CheckTransferCrossChainAssetTransaction(txn *Transaction) error {
	payloadObj, ok := txn.Payload.(*PayloadTransferCrossChainAsset)
	if !ok {
		return errors.New("Invalid transaction payload type")
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
		if _, exist := outputIndexMap[outputIndex]; exist {
			return errors.New("Invalid transaction payload cross chain index")
		}
		outputIndexMap[outputIndex] = struct{}{}
	}

	//check address in outputs and payload
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if bytes.Compare(txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash[0:1], []byte{PrefixCrossChain}) != 0 {
			return errors.New("Invalid transaction output address, without \"X\" at beginning")
		}
		if payloadObj.CrossChainAddresses[i] == "" {
			return errors.New("Invalid transaction cross chain address ")
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.CrossChainAmounts); i++ {
		if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] > txn.Outputs[payloadObj.OutputIndexes[i]].Value-Fixed64(config.Parameters.MinCrossChainTxFee) {
			return errors.New("Invalid transaction cross chain amount")
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
