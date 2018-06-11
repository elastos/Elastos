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
		log.Warn("[CheckTransactionAttribute],", err)
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

	if txn.IsSideminingTx() {
		arbitrtor, err := GetCurrentArbiter()
		if err != nil {
			return ErrSideMiningConsensus
		}
		if err = CheckSideMiningConsensus(txn, arbitrtor); err != nil {
			log.Warn("[CheckSideMiningConsensus],", err)
			return ErrSideMiningConsensus
		}
	}

	if txn.IsWithdrawFromSideChainTx() {
		witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
		for _, hash := range witPayload.SideChainTransactionHashes {
			if exist := DefaultLedger.Store.IsSidechainTxHashDuplicate(hash); exist {
				return ErrSidechainTxDuplicate
			}
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
		if referTxnOut.Value <= 0 {
			log.Warn("Value of referenced transaction output is invalid")
			return ErrInvalidReferedTx
		}
		// coinbase transaction only can be spent after got SpendCoinbaseSpan times confirmations
		if referTxn.IsCoinBaseTx() {
			lockHeight := referTxn.LockTime
			currentHeight := DefaultLedger.Store.GetHeight()
			if currentHeight-lockHeight < config.Parameters.ChainParam.SpendCoinbaseSpan {
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
		// FIXME coinbase reward should match 4% inflation per year
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
		if float64(foundationReward)/float64(totalReward) < 0.3 {
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
	if tx.IsWithdrawFromSideChainTx() {
		return nil
	}
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

func CheckAttributeProgram(txn *Transaction) error {
	//TODO: implement CheckAttributeProgram
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
	case *PayloadSideMining:
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
		existingHashs := make(map[string]struct{})
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

func CheckSideMiningConsensus(txn *Transaction, arbitrator []byte) error {
	payloadSideMining, ok := txn.Payload.(*PayloadSideMining)
	if !ok {
		return errors.New("Side mining transaction has invalid payload")
	}

	publicKey, err := DecodePoint(arbitrator)
	if err != nil {
		return err
	}

	buf := new(bytes.Buffer)
	err = payloadSideMining.Serialize(buf, SideMiningPayloadVersion)
	if err != nil {
		return err
	}

	err = Verify(*publicKey, buf.Bytes()[0:68], payloadSideMining.SignedData)
	if err != nil {
		return errors.New("Arbitrator is not matched")
	}

	return nil
}
