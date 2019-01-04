package blockchain

import (
	"bytes"
	"errors"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	. "github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/core/types/payload"
	. "github.com/elastos/Elastos.ELA/crypto"
	. "github.com/elastos/Elastos.ELA/errors"
)

const (
	MinDepositAmount    = 5000
	DepositLockupBlocks = 2160
	MaxStringLength     = 100
)

// CheckTransactionSanity verifys received single transaction
func CheckTransactionSanity(blockHeight uint32, txn *Transaction) ErrCode {
	if err := CheckTransactionSize(txn); err != nil {
		log.Warn("[CheckTransactionSize],", err)
		return ErrTransactionSize
	}

	if err := CheckTransactionInput(txn); err != nil {
		log.Warn("[CheckTransactionInput],", err)
		return ErrInvalidInput
	}

	if err := CheckTransactionOutput(blockHeight, txn); err != nil {
		log.Warn("[CheckTransactionOutput],", err)
		return ErrInvalidOutput
	}

	if err := CheckAssetPrecision(txn); err != nil {
		log.Warn("[CheckAssetPrecesion],", err)
		return ErrAssetPrecision
	}

	if err := CheckAttributeProgram(blockHeight, txn); err != nil {
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
func CheckTransactionContext(blockHeight uint32, txn *Transaction) ErrCode {
	// check if duplicated with transaction in ledger
	if exist := DefaultLedger.Store.IsTxHashDuplicate(txn.Hash()); exist {
		log.Warn("[CheckTransactionContext] duplicate transaction check failed.")
		return ErrTransactionDuplicate
	}

	if txn.IsCoinBaseTx() {
		return Success
	}

	if txn.IsIllegalProposalTx() {
		if err := CheckIllegalProposalsTransaction(txn); err != nil {
			log.Warn("[CheckIllegalProposalsTransaction],", err)
			return ErrTransactionPayload
		} else {
			return Success
		}
	}

	if txn.IsIllegalVoteTx() {
		if err := CheckIllegalVotesTransaction(txn); err != nil {
			log.Warn("[CheckIllegalVotesTransaction],", err)
			return ErrTransactionPayload
		} else {
			return Success
		}
	}

	if txn.IsIllegalBlockTx() {
		if err := CheckIllegalBlocksTransaction(txn); err != nil {
			log.Warn("[CheckIllegalBlocksTransaction],", err)
			return ErrTransactionPayload
		}
	}

	if txn.IsSideChainPowTx() {
		arbitrator := DefaultLedger.Arbitrators.GetOnDutyArbitrator()
		if err := CheckSideChainPowConsensus(txn, arbitrator); err != nil {
			log.Warn("[CheckSideChainPowConsensus],", err)
			return ErrSideChainPowConsensus
		}
	}

	if txn.IsRegisterProducerTx() {
		if err := CheckRegisterProducerTransaction(txn); err != nil {
			log.Warn("[CheckRegisterProducerTransaction],", err)
			return ErrTransactionPayload
		}
	}

	if txn.IsCancelProducerTx() {
		if err := CheckCancelProducerTransaction(txn); err != nil {
			log.Warn("[CheckCancelProducerTransaction],", err)
			return ErrTransactionPayload
		}
	}

	if txn.IsUpdateProducerTx() {
		if err := CheckUpdateProducerTransaction(txn); err != nil {
			log.Warn("[CheckUpdateProducerTransaction],", err)
			return ErrTransactionPayload
		}
	}

	if txn.IsReturnDepositCoin() {
		if err := CheckReturnDepositCoinTransaction(txn); err != nil {
			log.Warn("[CheckReturnDepositCoinTransaction],", err)
			return ErrReturnDepositConsensus
		}
	}

	// check double spent transaction
	if DefaultLedger.IsDoubleSpend(txn) {
		log.Warn("[CheckTransactionContext] IsDoubleSpend check faild.")
		return ErrDoubleSpend
	}

	references, err := DefaultLedger.Store.GetTxReference(txn)
	if err != nil {
		log.Warn("[CheckTransactionContext] get transaction reference failed")
		return ErrUnknownReferredTx
	}

	if txn.IsWithdrawFromSideChainTx() {
		if err := CheckWithdrawFromSideChainTransaction(txn, references); err != nil {
			log.Warn("[CheckWithdrawFromSideChainTransaction],", err)
			return ErrSidechainTxDuplicate
		}
	}

	if txn.IsTransferCrossChainAssetTx() {
		if err := CheckTransferCrossChainAssetTransaction(txn, references); err != nil {
			log.Warn("[CheckTransferCrossChainAssetTransaction],", err)
			return ErrInvalidOutput
		}
	}

	if err := CheckTransactionUTXOLock(txn, references); err != nil {
		log.Warn("[CheckTransactionUTXOLock],", err)
		return ErrUTXOLocked
	}

	if err := CheckTransactionFee(txn, references); err != nil {
		log.Warn("[CheckTransactionFee],", err)
		return ErrTransactionBalance
	}

	if err := CheckDestructionAddress(references); err != nil {
		log.Warn("[CheckDestructionAddress], ", err)
		return ErrInvalidInput
	}

	if err := CheckTransactionDepositUTXO(txn, references); err != nil {
		log.Warn("[CheckTransactionDepositUTXO],", err)
		return ErrInvalidInput
	}

	if err := CheckTransactionSignature(txn, references); err != nil {
		log.Warn("[CheckTransactionSignature],", err)
		return ErrTransactionSignature
	}

	if err := CheckTransactionCoinbaseOutputLock(txn); err != nil {
		log.Warn("[CheckTransactionCoinbaseLock]", err)
		return ErrIneffectiveCoinbase
	}

	if err := DefaultLedger.HeightVersions.CheckVoteProducerOutputs(blockHeight, txn, txn.Outputs, references,
		getProducerPublicKeys(DefaultLedger.Store.GetActiveRegisteredProducers())); err != nil {
		log.Warn("[CheckVoteProducerOutputs],", err)
		return ErrInvalidOutput
	}

	return Success
}

func getProducerPublicKeys(producers []*PayloadRegisterProducer) [][]byte {
	var publicKeys [][]byte
	for _, p := range producers {
		publicKeys = append(publicKeys, p.PublicKey)
	}
	return publicKeys
}

func CheckDestructionAddress(references map[*Input]*Output) error {
	for _, output := range references {
		// this uint168 code
		// is the program hash of the Elastos foundation destruction address ELANULLXXXXXXXXXXXXXXXXXXXXXYvs3rr
		// we allow no output from destruction address.
		// So add a check here in case someone crack the private key of this address.
		if output.ProgramHash == common.Uint168([21]uint8{33, 32, 254, 229, 215, 235, 62, 92, 125, 49, 151, 254, 207, 108, 13, 227, 15, 136, 154, 206, 247}) {
			return errors.New("cannot use utxo in the Elastos foundation destruction address")
		}
	}
	return nil
}

func CheckTransactionCoinbaseOutputLock(txn *Transaction) error {
	type lockTxInfo struct {
		isCoinbaseTx bool
		locktime     uint32
	}
	transactionCache := make(map[common.Uint256]lockTxInfo)
	currentHeight := DefaultLedger.Store.GetHeight()
	var referTxn *Transaction
	for _, input := range txn.Inputs {
		var lockHeight uint32
		var isCoinbase bool
		referHash := input.Previous.TxID
		if _, ok := transactionCache[referHash]; ok {
			lockHeight = transactionCache[referHash].locktime
			isCoinbase = transactionCache[referHash].isCoinbaseTx
		} else {
			var err error
			referTxn, _, err = DefaultLedger.Store.GetTransaction(referHash)
			// TODO
			// we have executed DefaultLedger.Store.GetTxReference(txn) before.
			//So if we can't find referTxn here, there must be a data inconsistent problem,
			// because we do not add lock correctly.This problem will be fixed later on.
			if err != nil {
				return errors.New("[CheckTransactionCoinbaseOutputLock] get tx reference failed:" + err.Error())
			}
			lockHeight = referTxn.LockTime
			isCoinbase = referTxn.IsCoinBaseTx()
			transactionCache[referHash] = lockTxInfo{isCoinbase, lockHeight}
		}

		if isCoinbase && currentHeight-lockHeight < config.Parameters.ChainParam.CoinbaseLockTime {
			return errors.New("cannot unlock coinbase transaction output")
		}
	}
	return nil
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
		if !coinbaseInputHash.IsEqual(common.EmptyHash) || coinbaseInputIndex != math.MaxUint16 {
			return errors.New("invalid coinbase input")
		}

		return nil
	}
	if txn.IsIllegalProposalTx() || txn.IsIllegalVoteTx() || txn.IsIllegalBlockTx() {
		if len(txn.Inputs) != 0 {
			return errors.New("illegal transactions must has no input")
		}
		return nil
	}

	if len(txn.Inputs) <= 0 {
		return errors.New("transaction has no inputs")
	}
	existingTxInputs := make(map[string]struct{})
	for _, input := range txn.Inputs {
		if input.Previous.TxID.IsEqual(common.EmptyHash) && (input.Previous.Index == math.MaxUint16) {
			return errors.New("invalid transaction input")
		}
		if _, exists := existingTxInputs[input.ReferKey()]; exists {
			return errors.New("duplicated transaction inputs")
		} else {
			existingTxInputs[input.ReferKey()] = struct{}{}
		}
	}

	return nil
}

func CheckTransactionOutput(blockHeight uint32, txn *Transaction) error {
	if len(txn.Outputs) > math.MaxUint16 {
		return errors.New("output count should not be greater than 65535(MaxUint16)")
	}

	for _, output := range txn.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit && !txn.IsRegisterProducerTx() {
			return errors.New("only RegisterProducer transaction can use the deposit address")
		}
	}

	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("coinbase output is not enough, at least 2")
		}

		if !txn.Outputs[0].ProgramHash.IsEqual(FoundationAddress) {
			return errors.New("First output address should be foundation address.")
		}

		var totalReward = common.Fixed64(0)
		for _, output := range txn.Outputs {
			if output.AssetID != DefaultLedger.Blockchain.AssetID {
				return errors.New("Asset ID in coinbase is invalid")
			}
			totalReward += output.Value
		}

		foundationReward := txn.Outputs[0].Value

		if common.Fixed64(foundationReward) < common.Fixed64(float64(totalReward)*0.3) {
			return errors.New("Reward to foundation in coinbase < 30%")
		}

		if err := DefaultLedger.HeightVersions.CheckCoinbaseMinerReward(blockHeight, txn, totalReward); err != nil {
			return err
		}

		return nil
	}
	if txn.IsIllegalProposalTx() || txn.IsIllegalVoteTx() || txn.IsIllegalBlockTx() {
		if len(txn.Outputs) != 0 {
			return errors.New("Illegal transactions should have no output")
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

		// output value must >= 0
		if output.Value < common.Fixed64(0) {
			return errors.New("Invalide transaction UTXO output.")
		}

		if err := DefaultLedger.HeightVersions.CheckOutputProgramHash(blockHeight, txn, output.ProgramHash); err != nil {
			return err
		}

		if err := DefaultLedger.HeightVersions.CheckOutputPayload(blockHeight, txn, output); err != nil {
			return err
		}
	}

	return nil
}

func CheckTransactionUTXOLock(txn *Transaction, references map[*Input]*Output) error {
	if txn.IsCoinBaseTx() {
		return nil
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

func CheckTransactionDepositUTXO(txn *Transaction, references map[*Input]*Output) error {
	for _, output := range references {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			if !txn.IsReturnDepositCoin() {
				return errors.New("only the ReturnDepositCoin transaction can use the deposit UTXO")
			}
		} else {
			if txn.IsReturnDepositCoin() {
				return errors.New("the ReturnDepositCoin transaction can only use the deposit UTXO")
			}
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
	assetOutputs := make(map[common.Uint256][]*Output)

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

func CheckTransactionFee(tx *Transaction, references map[*Input]*Output) error {
	var outputValue common.Fixed64
	var inputValue common.Fixed64
	for _, output := range tx.Outputs {
		outputValue += output.Value
	}
	for _, reference := range references {
		inputValue += reference.Value
	}
	if inputValue < common.Fixed64(config.Parameters.PowConfiguration.MinTxFee)+outputValue {
		return fmt.Errorf("transaction fee not enough")
	}
	return nil
}

func CheckAttributeProgram(blockHeight uint32, tx *Transaction) error {
	// Coinbase and illegal transactions do not check attribute and program
	if tx.IsCoinBaseTx() {
		return DefaultLedger.HeightVersions.CheckTxHasNoPrograms(blockHeight, tx)
	}

	if tx.IsIllegalProposalTx() || tx.IsIllegalVoteTx() {
		if len(tx.Programs) != 0 || len(tx.Attributes) != 0 {
			return errors.New("illegal proposal and vote transactions should have no attributes and programs")
		}
		return nil
	}

	if tx.IsIllegalBlockTx() {
		if len(tx.Programs) != 1 {
			return errors.New("illegal block transactions should have one and only one program")
		}
		if len(tx.Attributes) != 0 {
			return errors.New("illegal block transactions should have no programs")
		}
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
	}
	return nil
}

func CheckTransactionSignature(tx *Transaction, references map[*Input]*Output) error {
	programHashes, err := GetTxProgramHashes(tx, references)
	if err != nil {
		return err
	}

	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)

	// sort the program hashes of owner and programs of the transaction
	common.SortProgramHashByCodeHash(programHashes)
	SortPrograms(tx.Programs)

	return RunPrograms(buf.Bytes(), programHashes, tx.Programs)
}

func checkAmountPrecise(amount common.Fixed64, precision byte) bool {
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
	case *PayloadRegisterProducer:
	case *PayloadCancelProducer:
	case *PayloadUpdateProducer:
	case *PayloadReturnDepositCoin:
	default:
		return errors.New("[txValidator],invalidate transaction payload type.")
	}
	return nil
}

//validate the transaction of duplicate sidechain transaction
func CheckDuplicateSidechainTx(txn *Transaction) error {
	if txn.IsWithdrawFromSideChainTx() {
		witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)
		existingHashs := make(map[common.Uint256]struct{})
		for _, hash := range witPayload.SideChainTransactionHashes {
			if _, exist := existingHashs[hash]; exist {
				return errors.New("Duplicate sidechain tx detected in a transaction")
			}
			existingHashs[hash] = struct{}{}
		}
	}
	return nil
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

func CheckWithdrawFromSideChainTransaction(txn *Transaction, references map[*Input]*Output) error {
	witPayload, ok := txn.Payload.(*PayloadWithdrawFromSideChain)
	if !ok {
		return errors.New("Invalid withdraw from side chain payload type")
	}
	for _, hash := range witPayload.SideChainTransactionHashes {
		if exist := DefaultLedger.Store.IsSidechainTxHashDuplicate(hash); exist {
			return errors.New("Duplicate side chain transaction hash in paylod")
		}
	}

	for _, v := range references {
		if bytes.Compare(v.ProgramHash[0:1], []byte{common.PrefixCrossChain}) != 0 {
			return errors.New("Invalid transaction inputs address, without \"X\" at beginning")
		}
	}

	return nil
}

func CheckTransferCrossChainAssetTransaction(txn *Transaction, references map[*Input]*Output) error {
	payloadObj, ok := txn.Payload.(*PayloadTransferCrossChainAsset)
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
	csAddresses := make(map[string]struct{}, 0)
	for i := 0; i < len(payloadObj.CrossChainAddresses); i++ {
		if _, ok := csAddresses[payloadObj.CrossChainAddresses[i]]; ok {
			return errors.New("duplicated cross chain address in payload")
		}
		csAddresses[payloadObj.CrossChainAddresses[i]] = struct{}{}
		if bytes.Compare(txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash[0:1], []byte{common.PrefixCrossChain}) != 0 {
			return errors.New("Invalid transaction output address, without \"X\" at beginning")
		}
		if payloadObj.CrossChainAddresses[i] == "" {
			return errors.New("Invalid transaction cross chain address ")
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.CrossChainAmounts); i++ {
		if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] > txn.Outputs[payloadObj.OutputIndexes[i]].Value-common.Fixed64(config.Parameters.MinCrossChainTxFee) {
			return errors.New("Invalid transaction cross chain amount")
		}
	}

	//check transaction fee
	var totalInput common.Fixed64
	for _, v := range references {
		totalInput += v.Value
	}

	var totalOutput common.Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}

	if totalInput-totalOutput < common.Fixed64(config.Parameters.MinCrossChainTxFee) {
		return errors.New("Invalid transaction fee")
	}
	return nil
}

func CheckRegisterProducerTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadRegisterProducer)
	if !ok {
		return errors.New("invalid payload")
	}

	height, err := DefaultLedger.Store.GetCancelProducerHeight(payload.PublicKey)
	if err == nil {
		return fmt.Errorf("invalid producer, canceled at height: %d", height)
	}

	// check public key and nick name
	producers := DefaultLedger.Store.GetRegisteredProducers()
	hash, err := contract.PublicKeyToDepositProgramHash(payload.PublicKey)
	if err != nil {
		return errors.New("invalid public key")
	}
	if err := checkStringField(payload.NickName, "NickName"); err != nil {
		return err
	}
	for _, p := range producers {
		if bytes.Equal(p.PublicKey, payload.PublicKey) {
			return errors.New("duplicated public key")
		}
		if p.NickName == payload.NickName {
			return errors.New("duplicated nick name")
		}
	}

	// check url
	if err = checkStringField(payload.Url, "Url"); err != nil {
		return err
	}

	// check ip
	if err = checkStringField(payload.Address, "IP"); err != nil {
		return err
	}

	// check signature
	publicKey, err := DecodePoint(payload.PublicKey)
	if err != nil {
		return errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = payload.SerializeUnsigned(signedBuf, PayloadRegisterProducerVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), payload.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}

	// check the deposit coin
	var depositCount int
	for _, output := range txn.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			depositCount++
			if !output.ProgramHash.IsEqual(*hash) {
				return errors.New("deposit address does not match the public key in payload")
			}
			if output.Value < MinDepositAmount {
				return errors.New("the deposit coin is insufficient")
			}
		}
	}
	if depositCount != 1 {
		return errors.New("there must be only one deposit address in outputs")
	}

	return nil
}

func CheckCancelProducerTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadCancelProducer)
	if !ok {
		return errors.New("invalid payload")
	}

	// check signature
	publicKey, err := DecodePoint(payload.PublicKey)
	if err != nil {
		return errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = payload.SerializeUnsigned(signedBuf, PayloadCancelProducerVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), payload.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}

	producers := DefaultLedger.Store.GetRegisteredProducers()
	for _, p := range producers {
		if bytes.Equal(p.PublicKey, payload.PublicKey) {
			return nil
		}
	}
	return errors.New("invalid producer")
}

func CheckUpdateProducerTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadUpdateProducer)
	if !ok {
		return errors.New("invalid payload")
	}

	// check nick name
	if err := checkStringField(payload.NickName, "NickName"); err != nil {
		return err
	}

	// check url
	if err := checkStringField(payload.Url, "Url"); err != nil {
		return err
	}

	// check ip
	if err := checkStringField(payload.Address, "Ip"); err != nil {
		return err
	}

	// check signature
	publicKey, err := DecodePoint(payload.PublicKey)
	if err != nil {
		return errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = payload.SerializeUnsigned(signedBuf, PayloadUpdateProducerVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), payload.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}

	// check from database
	producers := DefaultLedger.Store.GetRegisteredProducers()
	hasProducer := false
	keepNickName := false
	for _, p := range producers {
		if bytes.Equal(p.PublicKey, payload.PublicKey) {
			hasProducer = true
			keepNickName = p.NickName == payload.NickName
			break
		}
	}
	if !hasProducer {
		return errors.New("invalid producer")
	}

	if !keepNickName {
		for _, p := range producers {
			if p.NickName == payload.NickName {
				return errors.New("duplicated nick name")
			}
		}
	}

	return nil
}

func CheckReturnDepositCoinTransaction(txn *Transaction) error {
	for _, program := range txn.Programs {
		cancelHeight, err := DefaultLedger.Store.GetCancelProducerHeight(program.Code[1 : len(program.Code)-1])
		if err != nil {
			return errors.New("no cancel sign found on this public key")
		}
		if DefaultLedger.Store.GetHeight()-cancelHeight < DepositLockupBlocks {
			return errors.New("the deposit does not meet the lockup limit")
		}
	}
	return nil
}

func CheckIllegalProposalsTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadIllegalProposal)
	if !ok {
		return errors.New("Invalid payload.")
	}

	return checkDposIllegalProposals(&payload.DposIllegalProposals)
}

func CheckIllegalVotesTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadIllegalVote)
	if !ok {
		return errors.New("Invalid payload.")
	}

	return checkDposIllegalVotes(&payload.DposIllegalVotes)
}

func CheckIllegalBlocksTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*PayloadIllegalBlock)
	if !ok {
		return errors.New("Invalid payload.")
	}

	return CheckDposIllegalBlocks(&payload.DposIllegalBlocks)
}

func checkDposIllegalProposals(d *DposIllegalProposals) error {
	if !d.Evidence.IsMatch() || !d.CompareEvidence.IsMatch() {
		return errors.New("proposal hash and block should match")
	}

	if d.Evidence.BlockHeader.Height != d.CompareEvidence.BlockHeader.Height {
		return errors.New("should be in same height")
	}

	if d.Evidence.Proposal.Hash().IsEqual(d.CompareEvidence.Proposal.Hash()) {
		return errors.New("proposals can not be same")
	}

	if d.Evidence.Proposal.Sponsor != d.CompareEvidence.Proposal.Sponsor {
		return errors.New("should be same sponsor")
	}

	if d.Evidence.Proposal.ViewOffset != d.Evidence.Proposal.ViewOffset {
		return errors.New("should in same view")
	}

	if !IsProposalValid(&d.Evidence.Proposal) || !IsProposalValid(&d.Evidence.Proposal) {
		return errors.New("proposal should be valid")
	}

	return nil
}

func checkDposIllegalVotes(d *DposIllegalVotes) error {

	if !d.Evidence.IsMatch() || !d.CompareEvidence.IsMatch() {
		return errors.New("vote, proposal and block should match")
	}

	if d.Evidence.BlockHeader.Height != d.CompareEvidence.BlockHeader.Height {
		return errors.New("should be in same height")
	}

	if d.Evidence.Vote.Hash().IsEqual(d.CompareEvidence.Vote.Hash()) {
		return errors.New("votes can not be same")
	}

	if d.Evidence.Vote.Signer != d.CompareEvidence.Vote.Signer {
		return errors.New("should be same signer")
	}

	if d.Evidence.Proposal.ViewOffset != d.CompareEvidence.Proposal.ViewOffset {
		return errors.New("should in same view")
	}

	if !IsProposalValid(&d.Evidence.Proposal) || IsProposalValid(&d.CompareEvidence.Proposal) ||
		!IsVoteValid(&d.Evidence.Vote) || IsVoteValid(&d.CompareEvidence.Vote) {
		return errors.New("votes and related proposals should be valid")
	}

	return nil
}

func CheckDposIllegalBlocks(d *DposIllegalBlocks) error {

	if d.Evidence.BlockHash().IsEqual(d.CompareEvidence.BlockHash()) {
		return errors.New("blocks can not be same")
	}

	if d.CoinType == ELACoin {
		var err error
		var header, compareHeader *Header
		var confirm, compareConfirm *DPosProposalVoteSlot

		if header, compareHeader, err = checkDposElaIllegalBlockHeaders(d); err != nil {
			return err
		}

		if confirm, compareConfirm, err = checkDposElaIllegalBlockConfirms(d, header, compareHeader); err != nil {
			return err
		}

		if err := checkDposElaIllegalBlockSigners(d, confirm, compareConfirm); err != nil {
			return err
		}
	}

	return nil
}

func checkDposElaIllegalBlockSigners(d *DposIllegalBlocks, confirm *DPosProposalVoteSlot, compareConfirm *DPosProposalVoteSlot) error {
	signers := d.Evidence.Signers
	compareSigners := d.CompareEvidence.Signers

	if uint32(len(signers)) < config.Parameters.ArbiterConfiguration.MajorityCount ||
		uint32(len(compareSigners)) < config.Parameters.ArbiterConfiguration.MajorityCount {
		return errors.New("Signers count less than dpos required majority count")
	}

	arbitratorsSet := make(map[string]interface{})
	for _, v := range DefaultLedger.Arbitrators.GetArbitrators() {
		arbitratorsSet[common.BytesToHexString(v)] = nil
	}

	for _, v := range signers {
		if _, ok := arbitratorsSet[common.BytesToHexString(v)]; !ok {
			return errors.New("Invalid signers within evidence.")
		}
	}

	for _, v := range compareSigners {
		if _, ok := arbitratorsSet[common.BytesToHexString(v)]; !ok {
			return errors.New("Invalid signers within evidence.")
		}
	}

	confirmSigners := getConfirmSigners(confirm)
	for _, v := range signers {
		if _, ok := confirmSigners[common.BytesToHexString(v)]; !ok {
			return errors.New("Signers and confirm votes do not match.")
		}
	}

	compareConfirmSigners := getConfirmSigners(compareConfirm)
	for _, v := range signers {
		if _, ok := compareConfirmSigners[common.BytesToHexString(v)]; !ok {
			return errors.New("Signers and confirm votes do not match.")
		}
	}

	return nil
}

func checkDposElaIllegalBlockConfirms(d *DposIllegalBlocks, header *Header, compareHeader *Header) (*DPosProposalVoteSlot, *DPosProposalVoteSlot, error) {
	confirm := &DPosProposalVoteSlot{}
	compareConfirm := &DPosProposalVoteSlot{}

	data := new(bytes.Buffer)
	data.Write(d.Evidence.BlockConfirm)
	if err := confirm.Deserialize(data); err != nil {
		return nil, nil, err
	}

	data = new(bytes.Buffer)
	data.Write(d.CompareEvidence.BlockConfirm)
	if err := compareConfirm.Deserialize(data); err != nil {
		return nil, nil, err
	}

	if err := CheckConfirm(confirm); err != nil {
		return nil, nil, err
	}

	if err := CheckConfirm(compareConfirm); err != nil {
		return nil, nil, err
	}

	if !confirm.Hash.IsEqual(header.Hash()) {
		return nil, nil, errors.New("block and related confirm do not match")
	}

	if !compareConfirm.Hash.IsEqual(compareHeader.Hash()) {
		return nil, nil, errors.New("block and related confirm do not match")
	}

	return confirm, compareConfirm, nil
}

func checkDposElaIllegalBlockHeaders(d *DposIllegalBlocks) (*Header, *Header, error) {
	header := &Header{}
	compareHeader := &Header{}

	data := new(bytes.Buffer)
	data.Write(d.Evidence.Block)
	if err := header.Deserialize(data); err != nil {
		return nil, nil, err
	}

	data = new(bytes.Buffer)
	data.Write(d.CompareEvidence.Block)
	if err := compareHeader.Deserialize(data); err != nil {
		return nil, nil, err
	}

	if header.Height != d.BlockHeight || compareHeader.Height != d.BlockHeight {
		return nil, nil, errors.New("block data is illegal")
	}

	//todo check header content

	return header, compareHeader, nil
}

func getConfirmSigners(confirm *DPosProposalVoteSlot) map[string]interface{} {
	result := make(map[string]interface{})
	for _, v := range confirm.Votes {
		result[v.Signer] = nil
	}
	return result
}

func checkStringField(rawStr string, field string) error {
	if len(rawStr) == 0 || len(rawStr) > MaxStringLength {
		return fmt.Errorf("Field %s has invalid string length.", field)
	}

	return nil
}
