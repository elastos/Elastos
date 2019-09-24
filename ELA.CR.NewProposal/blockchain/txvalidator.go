// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	crstate "github.com/elastos/Elastos.ELA/cr/state"
	"github.com/elastos/Elastos.ELA/crypto"
	. "github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/state"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/vm"
)

const (
	// MinDepositAmount is the minimum deposit as a producer.
	MinDepositAmount = 5000 * 100000000

	// DepositLockupBlocks indicates how many blocks need to wait when cancel
	// producer or CRC was triggered, and can submit return deposit coin request.
	DepositLockupBlocks = 2160

	// MaxStringLength is the maximum length of a string field.
	MaxStringLength = 100
)

// CheckTransactionSanity verifies received single transaction
func (b *BlockChain) CheckTransactionSanity(blockHeight uint32, txn *Transaction) ErrCode {
	if err := b.checkTxHeightVersion(txn, blockHeight); err != nil {
		return ErrTransactionHeightVersion
	}

	if err := checkTransactionSize(txn); err != nil {
		log.Warn("[CheckTransactionSize],", err)
		return ErrTransactionSize
	}

	if err := checkTransactionInput(txn); err != nil {
		log.Warn("[CheckTransactionInput],", err)
		return ErrInvalidInput
	}

	if err := b.checkTransactionOutput(blockHeight, txn); err != nil {
		log.Warn("[CheckTransactionOutput],", err)
		return ErrInvalidOutput
	}

	if err := checkAssetPrecision(txn); err != nil {
		log.Warn("[CheckAssetPrecesion],", err)
		return ErrAssetPrecision
	}

	if err := b.checkAttributeProgram(txn, blockHeight); err != nil {
		log.Warn("[CheckAttributeProgram],", err)
		return ErrAttributeProgram
	}

	if err := checkTransactionPayload(txn); err != nil {
		log.Warn("[CheckTransactionPayload],", err)
		return ErrTransactionPayload
	}

	if err := checkDuplicateSidechainTx(txn); err != nil {
		log.Warn("[CheckDuplicateSidechainTx],", err)
		return ErrSidechainTxDuplicate
	}

	return Success
}

// CheckTransactionContext verifies a transaction with history transaction in ledger
func (b *BlockChain) CheckTransactionContext(blockHeight uint32,
	txn *Transaction, references map[*Input]*Output) ErrCode {
	// check if duplicated with transaction in ledger
	if exist := b.db.IsTxHashDuplicate(txn.Hash()); exist {
		log.Warn("[CheckTransactionContext] duplicate transaction check failed.")
		return ErrTransactionDuplicate
	}

	switch txn.TxType {
	case CoinBase:
		return Success

	case IllegalProposalEvidence:
		if err := b.checkIllegalProposalsTransaction(txn); err != nil {
			log.Warn("[CheckIllegalProposalsTransaction],", err)
			return ErrTransactionPayload
		} else {
			return Success
		}

	case IllegalVoteEvidence:
		if err := b.checkIllegalVotesTransaction(txn); err != nil {
			log.Warn("[CheckIllegalVotesTransaction],", err)
			return ErrTransactionPayload
		}
		return Success

	case IllegalBlockEvidence:
		if err := b.checkIllegalBlocksTransaction(txn); err != nil {
			log.Warn("[CheckIllegalBlocksTransaction],", err)
			return ErrTransactionPayload
		}
		return Success

	case IllegalSidechainEvidence:
		if err := b.checkSidechainIllegalEvidenceTransaction(txn); err != nil {
			log.Warn("[CheckSidechainIllegalEvidenceTransaction],", err)
			return ErrTransactionPayload
		}
		return Success

	case InactiveArbitrators:
		if err := b.checkInactiveArbitratorsTransaction(txn); err != nil {
			log.Warn("[CheckInactiveArbitrators],", err)
			return ErrTransactionPayload
		}
		return Success

	case UpdateVersion:
		if err := b.checkUpdateVersionTransaction(txn); err != nil {
			log.Warn("[checkUpdateVersionTransaction],", err)
			return ErrTransactionPayload
		}
		return Success

	case SideChainPow:
		arbitrator := DefaultLedger.Arbitrators.GetOnDutyCrossChainArbitrator()
		if err := CheckSideChainPowConsensus(txn, arbitrator); err != nil {
			log.Warn("[CheckSideChainPowConsensus],", err)
			return ErrSideChainPowConsensus
		}
		if txn.IsNewSideChainPowTx() {
			return Success
		}

	case RegisterProducer:
		if err := b.checkRegisterProducerTransaction(txn); err != nil {
			log.Warn("[CheckRegisterProducerTransaction],", err)
			return ErrTransactionPayload
		}

	case CancelProducer:
		if err := b.checkCancelProducerTransaction(txn); err != nil {
			log.Warn("[CheckCancelProducerTransaction],", err)
			return ErrTransactionPayload
		}

	case UpdateProducer:
		if err := b.checkUpdateProducerTransaction(txn); err != nil {
			log.Warn("[CheckUpdateProducerTransaction],", err)
			return ErrTransactionPayload
		}

	case ActivateProducer:
		if err := b.checkActivateProducerTransaction(txn, blockHeight); err != nil {
			log.Warn("[CheckActivateProducerTransaction],", err)
			return ErrTransactionPayload
		}
		return Success

	case RegisterCR:
		if err := b.checkRegisterCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[checkRegisterCRTransaction],", err)
			return ErrTransactionPayload
		}

	case UpdateCR:
		if err := b.checkUpdateCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[ checkUpdateCRTransaction],", err)
			return ErrTransactionPayload
		}

	case UnregisterCR:
		if err := b.checkUnRegisterCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[checkUnRegisterCRTransaction],", err)
			return ErrTransactionPayload
		}
	}

	// check double spent transaction
	if DefaultLedger.IsDoubleSpend(txn) {
		log.Warn("[CheckTransactionContext] IsDoubleSpend check failed")
		return ErrDoubleSpend
	}

	if txn.IsWithdrawFromSideChainTx() {
		if err := b.checkWithdrawFromSideChainTransaction(txn, references); err != nil {
			log.Warn("[CheckWithdrawFromSideChainTransaction],", err)
			return ErrSidechainTxDuplicate
		}
	}

	if txn.IsTransferCrossChainAssetTx() {
		if err := b.checkTransferCrossChainAssetTransaction(txn, references); err != nil {
			log.Warn("[CheckTransferCrossChainAssetTransaction],", err)
			return ErrInvalidOutput
		}
	}

	if txn.IsReturnDepositCoin() {
		if err := b.checkReturnDepositCoinTransaction(
			txn, references, b.GetHeight()); err != nil {
			log.Warn("[CheckReturnDepositCoinTransaction],", err)
			return ErrReturnDepositConsensus
		}
	}

	if txn.IsReturnCRDepositCoinTx() {
		if err := b.checkReturnCRDepositCoinTransaction(
			txn, references, b.GetHeight(), b.crCommittee.IsInVotingPeriod); err != nil {
			log.Warn("[CheckReturnDepositCoinTransaction],", err)
			return ErrReturnDepositConsensus
		}
	}

	if err := checkTransactionUTXOLock(txn, references); err != nil {
		log.Warn("[CheckTransactionUTXOLock],", err)
		return ErrUTXOLocked
	}

	if err := b.checkTransactionFee(txn, references); err != nil {
		log.Warn("[CheckTransactionFee],", err)
		return ErrTransactionBalance
	}

	if err := checkDestructionAddress(references); err != nil {
		log.Warn("[CheckDestructionAddress], ", err)
		return ErrInvalidInput
	}

	if err := checkTransactionDepositUTXO(txn, references); err != nil {
		log.Warn("[CheckTransactionDepositUTXO],", err)
		return ErrInvalidInput
	}

	if err := checkTransactionDepositOutpus(b, txn); err != nil {
		log.Warn("[checkTransactionDepositOutpus],", err)
		return ErrInvalidOutput
	}

	if err := checkTransactionSignature(txn, references); err != nil {
		log.Warn("[CheckTransactionSignature],", err)
		return ErrTransactionSignature
	}

	if err := b.checkInvalidUTXO(txn); err != nil {
		log.Warn("[CheckTransactionCoinbaseLock]", err)
		return ErrIneffectiveCoinbase
	}

	if txn.Version >= TxVersion09 {
		producers := b.state.GetActiveProducers()
		if blockHeight < b.chainParams.PublicDPOSHeight {
			producers = append(producers, b.state.GetPendingCanceledProducers()...)
		}
		candidates := b.crCommittee.GetState().GetCandidates(crstate.Active)
		err := b.checkVoteOutputs(blockHeight, txn.Outputs, references,
			getProducerPublicKeysMap(producers), getCRCodesMap(candidates))
		if err != nil {
			log.Warn("[CheckVoteOutputs]", err)
			return ErrInvalidOutput
		}
	}

	return Success
}

func (b *BlockChain) checkVoteOutputs(blockHeight uint32, outputs []*Output, references map[*Input]*Output,
	pds map[string]struct{}, crs map[string]struct{}) error {
	programHashes := make(map[common.Uint168]struct{})
	for _, output := range references {
		programHashes[output.ProgramHash] = struct{}{}
	}
	for _, o := range outputs {
		if o.Type != OTVote {
			continue
		}
		if _, ok := programHashes[o.ProgramHash]; !ok {
			return errors.New("the output address of vote tx " +
				"should exist in its input")
		}
		payload, ok := o.Payload.(*outputpayload.VoteOutput)
		if !ok {
			return errors.New("invalid vote output payload")
		}
		for _, content := range payload.Contents {
			switch content.VoteType {
			case outputpayload.Delegate:
				err := b.checkVoteProducerContent(
					content, pds, payload.Version, o.Value)
				if err != nil {
					return err
				}
			case outputpayload.CRC:
				err := b.checkVoteCRContent(blockHeight,
					content, crs, payload.Version, o.Value)
				if err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (b *BlockChain) checkVoteProducerContent(content outputpayload.VoteContent,
	pds map[string]struct{}, payloadVersion byte, amount common.Fixed64) error {
	for _, cv := range content.CandidateVotes {
		if _, ok := pds[common.BytesToHexString(cv.Candidate)]; !ok {
			return fmt.Errorf("invalid vote output payload "+
				"producer candidate: %s", common.BytesToHexString(cv.Candidate))
		}
	}
	if payloadVersion >= outputpayload.VoteProducerAndCRVersion {
		for _, cv := range content.CandidateVotes {
			if cv.Votes > amount {
				return errors.New("votes larger than output amount")
			}
		}
	}

	return nil
}

func (b *BlockChain) checkVoteCRContent(blockHeight uint32, content outputpayload.VoteContent,
	crs map[string]struct{}, payloadVersion byte, amount common.Fixed64) error {

	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("cr vote tx must during voting period")
	}

	if payloadVersion < outputpayload.VoteProducerAndCRVersion {
		return errors.New("payload VoteProducerVersion not support vote CR")
	}
	for _, cv := range content.CandidateVotes {
		if _, ok := crs[common.BytesToHexString(cv.Candidate)]; !ok {
			return fmt.Errorf("invalid vote output payload "+
				"CR candidate: %s", common.BytesToHexString(cv.Candidate))
		}
	}
	var totalVotes common.Fixed64
	for _, cv := range content.CandidateVotes {
		totalVotes += cv.Votes
	}
	if totalVotes > amount {
		return errors.New("total votes larger than output amount")
	}

	return nil
}

func getProducerPublicKeysMap(producers []*state.Producer) map[string]struct{} {
	pds := make(map[string]struct{})
	for _, p := range producers {
		pds[common.BytesToHexString(p.Info().OwnerPublicKey)] = struct{}{}
	}
	return pds
}

func getCRCodesMap(crs []*crstate.Candidate) map[string]struct{} {
	codes := make(map[string]struct{})
	for _, c := range crs {
		codes[common.BytesToHexString(c.Info().Code)] = struct{}{}
	}
	return codes
}

func checkDestructionAddress(references map[*Input]*Output) error {
	for _, output := range references {
		if output.ProgramHash == config.DestructionAddress {
			return errors.New("cannot use utxo from the destruction address")
		}
	}
	return nil
}

func (b *BlockChain) checkInvalidUTXO(txn *Transaction) error {
	currentHeight := DefaultLedger.Blockchain.GetHeight()
	for _, input := range txn.Inputs {
		referTxn, err := b.UTXOCache.GetTransaction(input.Previous.TxID)
		if err != nil {
			return err
		}
		if referTxn.IsCoinBaseTx() {
			if currentHeight-referTxn.LockTime < b.chainParams.CoinbaseMaturity {
				return errors.New("the utxo of coinbase is locking")
			}
		} else if referTxn.IsNewSideChainPowTx() {
			return errors.New("cannot spend the utxo from a new sideChainPow tx")
		}
	}

	return nil
}

//validate the transaction of duplicate UTXO input
func checkTransactionInput(txn *Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Inputs) != 1 {
			return errors.New("coinbase must has only one input")
		}
		inputHash := txn.Inputs[0].Previous.TxID
		inputIndex := txn.Inputs[0].Previous.Index
		sequence := txn.Inputs[0].Sequence
		if !inputHash.IsEqual(common.EmptyHash) || inputIndex != math.MaxUint16 || sequence != math.MaxUint32 {
			return errors.New("invalid coinbase input")
		}

		return nil
	}

	if txn.IsIllegalTypeTx() || txn.IsInactiveArbitrators() ||
		txn.IsNewSideChainPowTx() || txn.IsUpdateVersion() ||
		txn.IsActivateProducerTx() {
		if len(txn.Inputs) != 0 {
			return errors.New("no cost transactions must has no input")
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

func (b *BlockChain) checkTransactionOutput(blockHeight uint32,
	txn *Transaction) error {
	if len(txn.Outputs) > math.MaxUint16 {
		return errors.New("output count should not be greater than 65535(MaxUint16)")
	}

	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("coinbase output is not enough, at least 2")
		}

		if !txn.Outputs[0].ProgramHash.IsEqual(FoundationAddress) {
			return errors.New("First output address should be foundation address.")
		}

		foundationReward := txn.Outputs[0].Value
		var totalReward = common.Fixed64(0)
		if blockHeight < b.chainParams.PublicDPOSHeight {
			for _, output := range txn.Outputs {
				if output.AssetID != config.ELAAssetID {
					return errors.New("Asset ID in coinbase is invalid")
				}
				totalReward += output.Value
			}

			if foundationReward < common.Fixed64(float64(totalReward)*0.3) {
				return errors.New("reward to foundation in coinbase < 30%")
			}
		} else {
			// check the ratio of FoundationAddress reward with miner reward
			totalReward = txn.Outputs[0].Value + txn.Outputs[1].Value
			if len(txn.Outputs) == 2 && foundationReward <
				common.Fixed64(float64(totalReward)*0.3/0.65) {
				return errors.New("reward to foundation in coinbase < 30%")
			}
		}

		return nil
	}

	if txn.IsIllegalTypeTx() || txn.IsInactiveArbitrators() ||
		txn.IsUpdateVersion() || txn.IsActivateProducerTx() {
		if len(txn.Outputs) != 0 {
			return errors.New("no cost transactions should have no output")
		}

		return nil
	}

	if txn.IsNewSideChainPowTx() {
		if len(txn.Outputs) != 1 {
			return errors.New("new sideChainPow tx must have only one output")
		}
		if txn.Outputs[0].Value != 0 {
			return errors.New("the value of new sideChainPow tx output must be 0")
		}
		if txn.Outputs[0].Type != OTNone {
			return errors.New("the type of new sideChainPow tx output must be OTNone")
		}

		return nil
	}

	if len(txn.Outputs) < 1 {
		return errors.New("transaction has no outputs")
	}
	// check if output address is valid
	specialOutputCount := 0
	for _, output := range txn.Outputs {
		if output.AssetID != config.ELAAssetID {
			return errors.New("asset ID in output is invalid")
		}

		// output value must >= 0
		if output.Value < common.Fixed64(0) {
			return errors.New("Invalide transaction UTXO output.")
		}

		if err := checkOutputProgramHash(blockHeight, output.ProgramHash); err != nil {
			return err
		}

		if txn.Version >= TxVersion09 {
			if output.Type != OTNone {
				specialOutputCount++
			}
			if err := checkOutputPayload(txn.TxType, output); err != nil {
				return err
			}
		}
	}
	if b.GetHeight() >= b.chainParams.PublicDPOSHeight && specialOutputCount > 1 {
		return errors.New("special output count should less equal than 1")
	}

	return nil
}

func checkOutputProgramHash(height uint32, programHash common.Uint168) error {
	// main version >= 88812
	if height >= config.DefaultParams.CheckAddressHeight {
		var empty = common.Uint168{}
		if programHash.IsEqual(empty) {
			return nil
		}

		prefix := contract.PrefixType(programHash[0])
		switch prefix {
		case contract.PrefixStandard:
		case contract.PrefixMultiSig:
		case contract.PrefixCrossChain:
		case contract.PrefixDeposit:
		default:
			return errors.New("invalid program hash prefix")
		}

		addr, err := programHash.ToAddress()
		if err != nil {
			return errors.New("invalid program hash")
		}
		_, err = common.Uint168FromAddress(addr)
		if err != nil {
			return errors.New("invalid program hash")
		}

		return nil
	}

	// old version [0, 88812)
	return nil
}

func checkOutputPayload(txType TxType, output *Output) error {
	// OTVote information can only be placed in TransferAsset transaction.
	if txType == TransferAsset {
		switch output.Type {
		case OTVote:
			if contract.GetPrefixType(output.ProgramHash) !=
				contract.PrefixStandard {
				return errors.New("output address should be standard")
			}
		case OTNone:
		case OTMapping:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	} else {
		switch output.Type {
		case OTNone:
		default:
			return errors.New("transaction type dose not match the output payload type")
		}
	}

	return output.Payload.Validate()
}

func checkTransactionUTXOLock(txn *Transaction, references map[*Input]*Output) error {
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

func checkTransactionDepositUTXO(txn *Transaction, references map[*Input]*Output) error {
	for _, output := range references {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			if !txn.IsReturnDepositCoin() && !txn.IsReturnCRDepositCoinTx() {
				return errors.New("only the ReturnDepositCoin and " +
					"ReturnCRDepositCoin transaction can use the deposit UTXO")
			}
		} else {
			if txn.IsReturnDepositCoin() || txn.IsReturnCRDepositCoinTx() {
				return errors.New("the ReturnDepositCoin and ReturnCRDepositCoin " +
					"transaction can only use the deposit UTXO")
			}
		}
	}

	return nil
}

func checkTransactionDepositOutpus(bc *BlockChain, txn *Transaction) error {
	for _, output := range txn.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			if txn.IsRegisterProducerTx() || txn.IsRegisterCRTx() {
				continue
			}
			if bc.state.ExistProducerByDID(output.ProgramHash) {
				continue
			}
			if bc.crCommittee.GetState().ExistCandidateByDID(output.ProgramHash) {
				continue
			}
			return errors.New("only the address that CR or Producer" +
				" registered can have the deposit UTXO")
		}
	}

	return nil
}

func checkTransactionSize(txn *Transaction) error {
	size := txn.GetSize()
	if size <= 0 || size > int(pact.MaxBlockSize) {
		return fmt.Errorf("Invalid transaction size: %d bytes", size)
	}

	return nil
}

func checkAssetPrecision(txn *Transaction) error {
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

func (b *BlockChain) checkTransactionFee(tx *Transaction, references map[*Input]*Output) error {
	var outputValue common.Fixed64
	var inputValue common.Fixed64
	for _, output := range tx.Outputs {
		outputValue += output.Value
	}
	for _, output := range references {
		inputValue += output.Value
	}
	if inputValue < b.chainParams.MinTransactionFee+outputValue {
		return fmt.Errorf("transaction fee not enough")
	}
	// set Fee and FeePerKB if check has passed
	tx.Fee = inputValue - outputValue
	buf := new(bytes.Buffer)
	tx.Serialize(buf)
	tx.FeePerKB = tx.Fee * 1000 / common.Fixed64(len(buf.Bytes()))
	return nil
}

func (b *BlockChain) checkAttributeProgram(tx *Transaction,
	blockHeight uint32) error {
	switch tx.TxType {
	case CoinBase:
		// Coinbase and illegal transactions do not check attribute and program
		if len(tx.Programs) != 0 {
			return errors.New("transaction should have no programs")
		}
		return nil
	case IllegalSidechainEvidence, IllegalProposalEvidence, IllegalVoteEvidence,
		ActivateProducer:
		if len(tx.Programs) != 0 || len(tx.Attributes) != 0 {
			return errors.New("zero cost tx should have no attributes and programs")
		}
		return nil
	case IllegalBlockEvidence:
		if len(tx.Programs) != 1 {
			return errors.New("illegal block transactions should have one and only one program")
		}
		if len(tx.Attributes) != 0 {
			return errors.New("illegal block transactions should have no programs")
		}
	case InactiveArbitrators, UpdateVersion:
		if len(tx.Programs) != 1 {
			return errors.New("inactive arbitrators transactions should have one and only one program")
		}
		if len(tx.Attributes) != 1 {
			return errors.New("inactive arbitrators transactions should have one and only one arbitrator")
		}
	case SideChainPow:
		if tx.IsNewSideChainPowTx() {
			if len(tx.Programs) != 0 || len(tx.Attributes) != 0 {
				return errors.New("sideChainPow transactions should have no attributes and programs")
			}
			return nil
		}
	case ReturnDepositCoin:
		if blockHeight >= b.chainParams.CRVotingStartHeight {
			if len(tx.Programs) != 1 {
				return errors.New("return deposit coin transactions should have one and only one program")
			}
		}
	case ReturnCRDepositCoin:
		if len(tx.Programs) != 1 {
			return errors.New("return CR deposit coin transactions should have one and only one program")
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

func checkTransactionSignature(tx *Transaction, references map[*Input]*Output) error {
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

func checkTransactionPayload(txn *Transaction) error {
	switch pld := txn.Payload.(type) {
	case *payload.RegisterAsset:
		if pld.Asset.Precision < payload.MinPrecision || pld.Asset.Precision > payload.MaxPrecision {
			return errors.New("Invalide asset Precision.")
		}
		if !checkAmountPrecise(pld.Amount, pld.Asset.Precision) {
			return errors.New("Invalide asset value,out of precise.")
		}
	case *payload.TransferAsset:
	case *payload.Record:
	case *payload.CoinBase:
	case *payload.SideChainPow:
	case *payload.WithdrawFromSideChain:
	case *payload.TransferCrossChainAsset:
	case *payload.ProducerInfo:
	case *payload.ProcessProducer:
	case *payload.ActivateProducer:
	case *payload.ReturnDepositCoin:
	case *payload.DPOSIllegalProposals:
	case *payload.DPOSIllegalVotes:
	case *payload.DPOSIllegalBlocks:
	case *payload.SidechainIllegalData:
	case *payload.InactiveArbitrators:
	case *payload.CRInfo:
	case *payload.UnregisterCR:

	default:
		return errors.New("[txValidator],invalidate transaction payload type.")
	}
	return nil
}

// validate the transaction of duplicate sidechain transaction
func checkDuplicateSidechainTx(txn *Transaction) error {
	if txn.IsWithdrawFromSideChainTx() {
		witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
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

// validate the type of transaction is allowed or not at current height.
func (b *BlockChain) checkTxHeightVersion(txn *Transaction, blockHeight uint32) error {
	switch txn.TxType {
	case RegisterCR, UpdateCR, UnregisterCR, ReturnCRDepositCoin:
		if blockHeight < b.chainParams.CRVotingStartHeight {
			return errors.New("not support before CRVotingStartHeight")
		}
	case TransferAsset:
		if blockHeight >= b.chainParams.CRVotingStartHeight {
			return nil
		}
		if txn.Version >= TxVersion09 {
			for _, output := range txn.Outputs {
				if output.Type != OTVote {
					continue
				}
				p, _ := output.Payload.(*outputpayload.VoteOutput)
				if p.Version >= outputpayload.VoteProducerAndCRVersion {
					return errors.New("not support " +
						"VoteProducerAndCRVersion before CRVotingStartHeight")
				}
			}
		}
	}

	return nil
}

func CheckSideChainPowConsensus(txn *Transaction, arbitrator []byte) error {
	payloadSideChainPow, ok := txn.Payload.(*payload.SideChainPow)
	if !ok {
		return errors.New("Side mining transaction has invalid payload")
	}

	publicKey, err := DecodePoint(arbitrator)
	if err != nil {
		return err
	}

	buf := new(bytes.Buffer)
	err = payloadSideChainPow.Serialize(buf, payload.SideChainPowVersion)
	if err != nil {
		return err
	}

	err = Verify(*publicKey, buf.Bytes()[0:68], payloadSideChainPow.Signature)
	if err != nil {
		return errors.New("Arbitrator is not matched. " + err.Error())
	}

	return nil
}

func (b *BlockChain) checkWithdrawFromSideChainTransaction(txn *Transaction, references map[*Input]*Output) error {
	witPayload, ok := txn.Payload.(*payload.WithdrawFromSideChain)
	if !ok {
		return errors.New("Invalid withdraw from side chain payload type")
	}
	for _, hash := range witPayload.SideChainTransactionHashes {
		if exist := DefaultLedger.Store.IsSidechainTxHashDuplicate(hash); exist {
			return errors.New("Duplicate side chain transaction hash in paylod")
		}
	}

	for _, output := range references {
		if bytes.Compare(output.ProgramHash[0:1], []byte{byte(contract.PrefixCrossChain)}) != 0 {
			return errors.New("Invalid transaction inputs address, without \"X\" at beginning")
		}
	}

	for _, p := range txn.Programs {
		publicKeys, err := crypto.ParseCrossChainScript(p.Code)
		if err != nil {
			return err
		}

		if err := b.checkCrossChainArbitrators(publicKeys); err != nil {
			return err
		}
	}

	return nil
}

func (b *BlockChain) checkCrossChainArbitrators(publicKeys [][]byte) error {
	arbiters := DefaultLedger.Arbitrators.GetCrossChainArbiters()
	if len(arbiters) != len(publicKeys) {
		return errors.New("invalid arbitrator count")
	}
	arbitratorsMap := make(map[string]interface{})
	for _, arbitrator := range arbiters {
		found := false
		for _, pk := range publicKeys {
			if bytes.Equal(arbitrator, pk[1:]) {
				found = true
				break
			}
		}

		if !found {
			return errors.New("invalid cross chain arbitrators")
		}

		arbitratorsMap[common.BytesToHexString(arbitrator)] = nil
	}

	if DefaultLedger.Blockchain.GetHeight()+1 >=
		b.chainParams.CRCOnlyDPOSHeight {
		for _, crc := range arbiters {
			if _, exist :=
				arbitratorsMap[common.BytesToHexString(crc)]; !exist {
				return errors.New("not all crc arbitrators participated in" +
					" crosschain multi-sign")
			}
		}
	}

	return nil
}

func (b *BlockChain) checkTransferCrossChainAssetTransaction(txn *Transaction, references map[*Input]*Output) error {
	payloadObj, ok := txn.Payload.(*payload.TransferCrossChainAsset)
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
		if bytes.Compare(txn.Outputs[payloadObj.OutputIndexes[i]].ProgramHash[0:1], []byte{byte(contract.PrefixCrossChain)}) != 0 {
			return errors.New("Invalid transaction output address, without \"X\" at beginning")
		}
		if payloadObj.CrossChainAddresses[i] == "" {
			return errors.New("Invalid transaction cross chain address ")
		}
	}

	//check cross chain amount in payload
	for i := 0; i < len(payloadObj.CrossChainAmounts); i++ {
		if payloadObj.CrossChainAmounts[i] < 0 || payloadObj.CrossChainAmounts[i] >
			txn.Outputs[payloadObj.OutputIndexes[i]].Value-b.chainParams.MinCrossChainTxFee {
			return errors.New("Invalid transaction cross chain amount")
		}
	}

	//check transaction fee
	var totalInput common.Fixed64
	for _, output := range references {
		totalInput += output.Value
	}

	var totalOutput common.Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}

	if totalInput-totalOutput < b.chainParams.MinCrossChainTxFee {
		return errors.New("Invalid transaction fee")
	}
	return nil
}

func (b *BlockChain) checkRegisterProducerTransaction(txn *Transaction) error {
	info, ok := txn.Payload.(*payload.ProducerInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	if err := checkStringField(info.NickName, "NickName"); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url"); err != nil {
		return err
	}

	if b.GetHeight() < b.chainParams.PublicDPOSHeight {
		// check duplication of node.
		if b.state.ProducerExists(info.NodePublicKey) {
			return fmt.Errorf("producer already registered")
		}

		// check duplication of owner.
		if b.state.ProducerExists(info.OwnerPublicKey) {
			return fmt.Errorf("producer owner already registered")
		}
	} else {
		// check duplication of node.
		if b.state.ProducerNodePublicKeyExists(info.NodePublicKey) {
			return fmt.Errorf("producer already registered")
		}

		// check duplication of owner.
		if b.state.ProducerOwnerPublicKeyExists(info.OwnerPublicKey) {
			return fmt.Errorf("producer owner already registered")
		}
	}

	// check duplication of nickname.
	if b.state.NicknameExists(info.NickName) {
		return fmt.Errorf("nick name %s already inuse", info.NickName)
	}

	// check if public keys conflict with cr program code
	ownerCode := append([]byte{byte(COMPRESSEDLEN)}, info.OwnerPublicKey...)
	ownerCode = append(ownerCode, vm.CHECKSIG)
	if b.crCommittee.ExistCR(ownerCode) {
		return fmt.Errorf("owner public key %s already exist in cr list",
			common.BytesToHexString(info.OwnerPublicKey))
	}
	nodeCode := append([]byte{byte(COMPRESSEDLEN)}, info.NodePublicKey...)
	nodeCode = append(nodeCode, vm.CHECKSIG)
	if b.crCommittee.ExistCR(nodeCode) {
		return fmt.Errorf("node public key %s already exist in cr list",
			common.BytesToHexString(info.NodePublicKey))
	}

	if err := b.additionalProducerInfoCheck(info); err != nil {
		return err
	}

	// check signature
	publicKey, err := DecodePoint(info.OwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = info.SerializeUnsigned(signedBuf, payload.ProducerInfoVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), info.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}

	// check the deposit coin
	hash, err := contract.PublicKeyToDepositProgramHash(info.OwnerPublicKey)
	if err != nil {
		return errors.New("invalid public key")
	}
	var depositCount int
	for _, output := range txn.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			depositCount++
			if !output.ProgramHash.IsEqual(*hash) {
				return errors.New("deposit address does not match the public key in payload")
			}
			if output.Value < MinDepositAmount {
				return errors.New("producer deposit amount is insufficient")
			}
		}
	}
	if depositCount != 1 {
		return errors.New("there must be only one deposit address in outputs")
	}

	return nil
}

func (b *BlockChain) checkProcessProducer(txn *Transaction) (
	*state.Producer, error) {
	processProducer, ok := txn.Payload.(*payload.ProcessProducer)
	if !ok {
		return nil, errors.New("invalid payload")
	}

	// check signature
	publicKey, err := DecodePoint(processProducer.OwnerPublicKey)
	if err != nil {
		return nil, errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = processProducer.SerializeUnsigned(signedBuf, payload.ProcessProducerVersion)
	if err != nil {
		return nil, err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), processProducer.Signature)
	if err != nil {
		return nil, errors.New("invalid signature in payload")
	}

	producer := b.state.GetProducer(processProducer.OwnerPublicKey)
	if producer == nil || !bytes.Equal(producer.OwnerPublicKey(),
		processProducer.OwnerPublicKey) {
		return nil, errors.New("getting unknown producer")
	}
	return producer, nil
}

func (b *BlockChain) checkActivateProducer(txn *Transaction) (
	*state.Producer, error) {
	activateProducer, ok := txn.Payload.(*payload.ActivateProducer)
	if !ok {
		return nil, errors.New("invalid payload")
	}

	// check signature
	publicKey, err := DecodePoint(activateProducer.NodePublicKey)
	if err != nil {
		return nil, errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = activateProducer.SerializeUnsigned(signedBuf, payload.ActivateProducerVersion)
	if err != nil {
		return nil, err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), activateProducer.Signature)
	if err != nil {
		return nil, errors.New("invalid signature in payload")
	}

	producer := b.state.GetProducer(activateProducer.NodePublicKey)
	if producer == nil || !bytes.Equal(producer.NodePublicKey(),
		activateProducer.NodePublicKey) {
		return nil, errors.New("getting unknown producer")
	}
	return producer, nil
}

func (b *BlockChain) checkCancelProducerTransaction(txn *Transaction) error {
	producer, err := b.checkProcessProducer(txn)
	if err != nil {
		return err
	}

	if producer.State() == state.Illegal ||
		producer.State() == state.Canceled {
		return errors.New("can not cancel this producer")
	}

	return nil
}

func (b *BlockChain) checkActivateProducerTransaction(txn *Transaction,
	height uint32) error {
	producer, err := b.checkActivateProducer(txn)
	if err != nil {
		return err
	}

	if height < b.chainParams.EnableActivateIllegalHeight {
		if producer.State() != state.Inactive {
			return errors.New("can not activate this producer")
		}
	} else {
		if producer.State() != state.Inactive &&
			producer.State() != state.Illegal {
			return errors.New("can not activate this producer")
		}
	}

	if height > producer.ActivateRequestHeight() &&
		height-producer.ActivateRequestHeight() <= state.ActivateDuration {
		return errors.New("can only activate once during inactive state")
	}

	depositAmount := common.Fixed64(0)
	if height < b.chainParams.CRVotingStartHeight {
		programHash, err := contract.PublicKeyToDepositProgramHash(
			producer.OwnerPublicKey())
		if err != nil {
			return err
		}

		utxos, err := b.db.GetUnspentFromProgramHash(*programHash, config.ELAAssetID)
		if err != nil {
			return err
		}

		for _, u := range utxos {
			depositAmount += u.Value
		}
	} else {
		depositAmount = producer.DepositAmount()
	}

	if depositAmount-producer.Penalty() < MinDepositAmount {
		return errors.New("insufficient deposit amount")
	}

	return nil
}

func (b *BlockChain) checkUpdateProducerTransaction(txn *Transaction) error {
	info, ok := txn.Payload.(*payload.ProducerInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	// check nick name
	if err := checkStringField(info.NickName, "NickName"); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url"); err != nil {
		return err
	}

	if err := b.additionalProducerInfoCheck(info); err != nil {
		return err
	}

	// check signature
	publicKey, err := DecodePoint(info.OwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = info.SerializeUnsigned(signedBuf, payload.ProducerInfoVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), info.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}

	producer := b.state.GetProducer(info.OwnerPublicKey)
	if producer == nil {
		return errors.New("updating unknown producer")
	}

	// check nickname usage.
	if producer.Info().NickName != info.NickName &&
		b.state.NicknameExists(info.NickName) {
		return fmt.Errorf("nick name %s already exist", info.NickName)
	}

	// check if public keys conflict with cr program code
	nodeCode := append([]byte{byte(COMPRESSEDLEN)}, info.NodePublicKey...)
	nodeCode = append(nodeCode, vm.CHECKSIG)
	if b.crCommittee.ExistCR(nodeCode) {
		return fmt.Errorf("node public key %s already exist in cr list",
			common.BytesToHexString(info.NodePublicKey))
	}

	// check node public key duplication
	if bytes.Equal(info.NodePublicKey, producer.Info().NodePublicKey) {
		return nil
	}

	if b.GetHeight() < b.chainParams.PublicDPOSHeight {
		if b.state.ProducerExists(info.NodePublicKey) {
			return fmt.Errorf("producer %s already exist",
				hex.EncodeToString(info.NodePublicKey))
		}
	} else {
		if b.state.ProducerNodePublicKeyExists(info.NodePublicKey) {
			return fmt.Errorf("producer %s already exist",
				hex.EncodeToString(info.NodePublicKey))
		}
	}

	return nil
}

func (b *BlockChain) checkRegisterCRTransaction(txn *Transaction,
	blockHeight uint32) error {
	info, ok := txn.Payload.(*payload.CRInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	if err := checkStringField(info.NickName, "NickName"); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url"); err != nil {
		return err
	}

	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("should create tx during voting period")
	}

	if b.crCommittee.GetState().ExistCandidateByNickname(info.NickName) {
		return fmt.Errorf("nick name %s already inuse", info.NickName)
	}

	cr := b.crCommittee.GetState().GetCandidate(info.Code)
	if cr != nil && cr.State() != crstate.Returned {
		return fmt.Errorf("did %s already exist", info.DID)
	}

	// get DID program hash
	ct, err := contract.CreateCRDIDContractByCode(info.Code)
	if err != nil {
		return err
	}
	programHash := ct.ToProgramHash()

	// check if program code conflict with producer public keys
	if info.Code[len(info.Code)-1] == vm.CHECKSIG {
		pk := info.Code[1 : len(info.Code)-1]
		if b.state.ProducerExists(pk) {
			return fmt.Errorf("public key %s already inuse in producer list",
				common.BytesToHexString(info.Code[1:len(info.Code)-1]))
		}
		if DefaultLedger.Arbitrators.IsCRCArbitrator(pk) {
			return fmt.Errorf("public key %s already inuse in CRC list",
				common.BytesToHexString(info.Code[0:len(info.Code)-1]))
		}
	}

	// check DID
	if !info.DID.IsEqual(*programHash) {
		return errors.New("invalid did address")
	}

	// check code and signature
	if err := b.crInfoSanityCheck(info); err != nil {
		return err
	}

	// check the deposit coin
	var depositCount int
	for _, output := range txn.Outputs {
		if contract.GetPrefixType(output.ProgramHash) == contract.PrefixDeposit {
			depositCount++
			// get deposit program hash
			ct, err := contract.CreateDepositContractByCode(info.Code)
			if err != nil {
				return err
			}
			programHash := ct.ToProgramHash()
			if !output.ProgramHash.IsEqual(*programHash) {
				return errors.New("deposit address does not" +
					" match the code in payload")
			}
			if output.Value < MinDepositAmount {
				return errors.New("CR deposit amount is insufficient")
			}
		}
	}
	if depositCount != 1 {
		return errors.New("there must be only one deposit address in outputs")
	}

	return nil
}

func (b *BlockChain) checkUpdateCRTransaction(txn *Transaction,
	blockHeight uint32) error {
	info, ok := txn.Payload.(*payload.CRInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	if err := checkStringField(info.NickName, "NickName"); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url"); err != nil {
		return err
	}

	// get did program hash
	ct, err := contract.CreateCRDIDContractByCode(info.Code)
	if err != nil {
		return err
	}
	programHash := ct.ToProgramHash()
	if err != nil {
		return err
	}

	// check DID
	if !info.DID.IsEqual(*programHash) {
		return errors.New("invalid did address")
	}

	// check code and signature
	if err := b.crInfoSanityCheck(info); err != nil {
		return err
	}
	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("should create tx during voting period")
	}

	cr := b.crCommittee.GetState().GetCandidate(info.Code)
	if cr == nil {
		return errors.New("updating unknown CR")
	}
	if cr.State() != crstate.Pending && cr.State() != crstate.Active {
		return errors.New("updating canceled CR")
	}

	// check nickname usage.
	if cr.Info().NickName != info.NickName &&
		b.crCommittee.GetState().ExistCandidateByNickname(info.NickName) {
		return fmt.Errorf("nick name %s already exist", info.NickName)
	}

	return nil
}

func (b *BlockChain) checkUnRegisterCRTransaction(txn *Transaction,
	blockHeight uint32) error {
	info, ok := txn.Payload.(*payload.UnregisterCR)
	if !ok {
		return errors.New("invalid payload")
	}

	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("should create tx during voting period")
	}

	cr := b.crCommittee.GetState().GetCandidate(info.Code)
	if cr == nil {
		return errors.New("unregister unknown CR")
	}
	if cr.State() != crstate.Pending && cr.State() != crstate.Active {
		return errors.New("unregister canceled CR")
	}

	signedBuf := new(bytes.Buffer)
	err := info.SerializeUnsigned(signedBuf, payload.UnregisterCRVersion)
	if err != nil {
		return err
	}
	return checkCRTransactionSignature(info.Signature, info.Code, signedBuf.Bytes())
}

func getParameterBySignature(signature []byte) []byte {
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)
	return buf.Bytes()
}

func checkCRTransactionSignature(signature []byte, code []byte, data []byte) error {
	signType, err := crypto.GetScriptType(code)
	if err != nil {
		return errors.New("invalid code")
	}
	if signType == vm.CHECKSIG {
		// check code and signature
		if err := checkStandardSignature(program.Program{
			Code:      code,
			Parameter: getParameterBySignature(signature),
		}, data); err != nil {
			return err
		}
	} else if signType == vm.CHECKMULTISIG {
		return errors.New("CR not support multi sign code")

		// check code and signature
		if err := checkMultiSigSignatures(program.Program{
			Code:      code,
			Parameter: signature,
		}, data); err != nil {
			return err
		}
	} else {
		return errors.New("invalid code type")
	}

	return nil

}

func (b *BlockChain) crInfoSanityCheck(info *payload.CRInfo) error {
	signedBuf := new(bytes.Buffer)
	err := info.SerializeUnsigned(signedBuf, payload.CRInfoVersion)
	if err != nil {
		return err
	}
	return checkCRTransactionSignature(info.Signature, info.Code, signedBuf.Bytes())
}

func (b *BlockChain) additionalProducerInfoCheck(
	info *payload.ProducerInfo) error {
	if b.GetHeight() >= b.chainParams.PublicDPOSHeight {
		_, err := DecodePoint(info.NodePublicKey)
		if err != nil {
			return errors.New("invalid node public key in payload")
		}

		if DefaultLedger.Arbitrators.IsCRCArbitrator(info.NodePublicKey) {
			return errors.New("node public key can't equal with CRC")
		}

		if DefaultLedger.Arbitrators.IsCRCArbitrator(info.OwnerPublicKey) {
			return errors.New("owner public key can't equal with CRC")
		}
	}
	return nil
}

func (b *BlockChain) checkReturnDepositCoinTransaction(txn *Transaction,
	references map[*Input]*Output, currentHeight uint32) error {

	var outputValue common.Fixed64
	var inputValue common.Fixed64
	for _, output := range txn.Outputs {
		outputValue += output.Value
	}
	for _, output := range references {
		inputValue += output.Value
	}

	var penalty common.Fixed64
	for _, program := range txn.Programs {
		p := b.state.GetProducer(program.Code[1 : len(program.Code)-1])
		if p == nil {
			return errors.New("signer must be producer")
		}
		if p.State() != state.Canceled {
			return errors.New("producer must be canceled before return deposit coin")
		}
		if currentHeight-p.CancelHeight() < DepositLockupBlocks {
			return errors.New("return deposit does not meet the lockup limit")
		}
		penalty += p.Penalty()
	}

	if inputValue-penalty < b.chainParams.MinTransactionFee+outputValue {
		return fmt.Errorf("overspend deposit")
	}

	return nil
}

func (b *BlockChain) checkReturnCRDepositCoinTransaction(txn *Transaction,
	references map[*Input]*Output, currentHeight uint32,
	isInVotingPeriod func(height uint32) bool) error {

	var outputValue common.Fixed64
	var inputValue common.Fixed64
	for _, output := range txn.Outputs {
		outputValue += output.Value
	}
	for _, output := range references {
		inputValue += output.Value
	}

	var penalty common.Fixed64
	for _, program := range txn.Programs {
		// Get candidate from code.
		ct, err := contract.CreateCRDIDContractByCode(program.Code)
		if err != nil {
			return err
		}
		programHash := ct.ToProgramHash()
		// todo get candidate from not voting period state.
		c := b.crCommittee.GetState().GetCandidateByDID(*programHash)
		if c == nil {
			return errors.New("signer must be CR candidate")
		}

		if isInVotingPeriod(currentHeight) {
			// In voting period, state need to be canceled.
			if c.State() != crstate.Canceled {
				return errors.New("candidate state is not canceled")
			}
			// In voting period, need to wait 720*3 blocks before return
			// deposit coin.
			if currentHeight-c.CancelHeight() < DepositLockupBlocks {
				return errors.New("return CR deposit does not " +
					"meet the lockup limit")
			}
		} else {
			// Not in voting period, state can be pending active or canceled
			// and no need to wait 720*3 blocks.
			if c.State() == crstate.Returned {
				return errors.New("candidate is returned before")
			}
		}
		penalty += c.Penalty()
	}

	// Check output amount.
	if inputValue-penalty < b.chainParams.MinTransactionFee+outputValue {
		return fmt.Errorf("candidate overspend deposit")
	}

	return nil
}

func (b *BlockChain) checkIllegalProposalsTransaction(txn *Transaction) error {
	p, ok := txn.Payload.(*payload.DPOSIllegalProposals)
	if !ok {
		return errors.New("invalid payload")
	}

	if b.state.SpecialTxExists(txn) {
		return errors.New("tx already exists")
	}

	return CheckDPOSIllegalProposals(p)
}

func (b *BlockChain) checkIllegalVotesTransaction(txn *Transaction) error {
	p, ok := txn.Payload.(*payload.DPOSIllegalVotes)
	if !ok {
		return errors.New("invalid payload")
	}

	if b.state.SpecialTxExists(txn) {
		return errors.New("tx already exists")
	}

	return CheckDPOSIllegalVotes(p)
}

func (b *BlockChain) checkIllegalBlocksTransaction(txn *Transaction) error {
	p, ok := txn.Payload.(*payload.DPOSIllegalBlocks)
	if !ok {
		return errors.New("invalid payload")
	}

	if b.state.SpecialTxExists(txn) {
		return errors.New("tx already exists")
	}

	return CheckDPOSIllegalBlocks(p)
}

func (b *BlockChain) checkInactiveArbitratorsTransaction(
	txn *Transaction) error {

	if b.state.SpecialTxExists(txn) {
		return errors.New("tx already exists")
	}

	return CheckInactiveArbitrators(txn)
}

func (b *BlockChain) checkUpdateVersionTransaction(txn *Transaction) error {
	payload, ok := txn.Payload.(*payload.UpdateVersion)
	if !ok {
		return errors.New("invalid payload")
	}

	if payload.EndHeight <= payload.StartHeight ||
		payload.StartHeight < b.GetHeight() {
		return errors.New("invalid update version height")
	}

	return checkCRCArbitratorsSignatures(txn.Programs[0])
}

func (b *BlockChain) checkSidechainIllegalEvidenceTransaction(txn *Transaction) error {
	p, ok := txn.Payload.(*payload.SidechainIllegalData)
	if !ok {
		return errors.New("invalid payload")
	}

	if b.state.SpecialTxExists(txn) {
		return errors.New("tx already exists")
	}

	return CheckSidechainIllegalEvidence(p)
}

func CheckSidechainIllegalEvidence(p *payload.SidechainIllegalData) error {

	if p.IllegalType != payload.SidechainIllegalProposal &&
		p.IllegalType != payload.SidechainIllegalVote {
		return errors.New("invalid type")
	}

	_, err := crypto.DecodePoint(p.IllegalSigner)
	if err != nil {
		return err
	}

	if !DefaultLedger.Arbitrators.IsArbitrator(p.IllegalSigner) {
		return errors.New("illegal signer is not one of current arbitrators")
	}

	_, err = common.Uint168FromAddress(p.GenesisBlockAddress)
	// todo check genesis block when sidechain registered in the future
	if err != nil {
		return err
	}

	if len(p.Signs) <= int(DefaultLedger.Arbitrators.GetArbitersMajorityCount()) {
		return errors.New("insufficient signs count")
	}

	if p.Evidence.DataHash.Compare(p.CompareEvidence.DataHash) >= 0 {
		return errors.New("evidence order error")
	}

	//todo get arbitrators by payload.Height and verify each sign in signs

	return nil
}

func CheckInactiveArbitrators(txn *Transaction) error {
	p, ok := txn.Payload.(*payload.InactiveArbitrators)
	if !ok {
		return errors.New("invalid payload")
	}

	if !DefaultLedger.Arbitrators.IsCRCArbitrator(p.Sponsor) {
		return errors.New("sponsor is not belong to arbitrators")
	}

	for _, v := range p.Arbitrators {
		if !DefaultLedger.Arbitrators.IsActiveProducer(v) &&
			!DefaultLedger.Arbitrators.IsDisabledProducer(v) {
			return errors.New("inactive arbitrator is not belong to " +
				"arbitrators")
		}
		if DefaultLedger.Arbitrators.IsCRCArbitrator(v) {
			return errors.New("inactive arbiters should not include CRC")
		}
	}

	if err := checkCRCArbitratorsSignatures(txn.Programs[0]); err != nil {
		return err
	}

	return nil
}

func checkCRCArbitratorsSignatures(program *program.Program) error {

	code := program.Code
	// Get N parameter
	n := int(code[len(code)-2]) - crypto.PUSH1 + 1
	// Get M parameter
	m := int(code[0]) - crypto.PUSH1 + 1

	crcArbitrators := DefaultLedger.Arbitrators.GetCRCArbitrators()
	crcArbitratorsCount := len(crcArbitrators)
	minSignCount := int(float64(crcArbitratorsCount)*
		state.MajoritySignRatioNumerator/state.MajoritySignRatioDenominator) + 1
	if m < 1 || m > n || n != crcArbitratorsCount || m < minSignCount {
		fmt.Printf("m:%d n:%d minSignCount:%d crc:  %d", m, n, minSignCount, crcArbitratorsCount)
		return errors.New("invalid multi sign script code")
	}
	publicKeys, err := crypto.ParseMultisigScript(code)
	if err != nil {
		return err
	}

	for _, pk := range publicKeys {
		str := common.BytesToHexString(pk[1:])
		if _, exists := crcArbitrators[str]; !exists {
			return errors.New("invalid multi sign public key")
		}
	}
	return nil
}

func CheckDPOSIllegalProposals(d *payload.DPOSIllegalProposals) error {

	if err := validateProposalEvidence(&d.Evidence); err != nil {
		return err
	}

	if err := validateProposalEvidence(&d.CompareEvidence); err != nil {
		return err
	}

	if d.Evidence.BlockHeight != d.CompareEvidence.BlockHeight {
		return errors.New("should be in same height")
	}

	if d.Evidence.Proposal.Hash().IsEqual(d.CompareEvidence.Proposal.Hash()) {
		return errors.New("proposals can not be same")
	}

	if d.Evidence.Proposal.Hash().Compare(
		d.CompareEvidence.Proposal.Hash()) > 0 {
		return errors.New("evidence order error")
	}

	if !bytes.Equal(d.Evidence.Proposal.Sponsor, d.CompareEvidence.Proposal.Sponsor) {
		return errors.New("should be same sponsor")
	}

	if d.Evidence.Proposal.ViewOffset != d.CompareEvidence.Proposal.ViewOffset {
		return errors.New("should in same view")
	}

	if err := ProposalCheckByHeight(&d.Evidence.Proposal, d.GetBlockHeight()); err != nil {
		return err
	}

	if err := ProposalCheckByHeight(&d.CompareEvidence.Proposal,
		d.GetBlockHeight()); err != nil {
		return err
	}

	return nil
}

func CheckDPOSIllegalVotes(d *payload.DPOSIllegalVotes) error {

	if err := validateVoteEvidence(&d.Evidence); err != nil {
		return err
	}

	if err := validateVoteEvidence(&d.CompareEvidence); err != nil {
		return err
	}

	if d.Evidence.BlockHeight != d.CompareEvidence.BlockHeight {
		return errors.New("should be in same height")
	}

	if d.Evidence.Vote.Hash().IsEqual(d.CompareEvidence.Vote.Hash()) {
		return errors.New("votes can not be same")
	}

	if d.Evidence.Vote.Hash().Compare(d.CompareEvidence.Vote.Hash()) > 0 {
		return errors.New("evidence order error")
	}

	if !bytes.Equal(d.Evidence.Vote.Signer, d.CompareEvidence.Vote.Signer) {
		return errors.New("should be same signer")
	}

	if !bytes.Equal(d.Evidence.Proposal.Sponsor, d.CompareEvidence.Proposal.Sponsor) {
		return errors.New("should be same sponsor")
	}

	if d.Evidence.Proposal.ViewOffset != d.CompareEvidence.Proposal.ViewOffset {
		return errors.New("should in same view")
	}

	if err := ProposalCheckByHeight(&d.Evidence.Proposal,
		d.GetBlockHeight()); err != nil {
		return err
	}

	if err := ProposalCheckByHeight(&d.CompareEvidence.Proposal,
		d.GetBlockHeight()); err != nil {
		return err
	}

	if err := VoteCheckByHeight(&d.Evidence.Vote,
		d.GetBlockHeight()); err != nil {
		return err
	}

	if err := VoteCheckByHeight(&d.CompareEvidence.Vote,
		d.GetBlockHeight()); err != nil {
		return err
	}

	return nil
}

func CheckDPOSIllegalBlocks(d *payload.DPOSIllegalBlocks) error {

	if d.Evidence.BlockHash().IsEqual(d.CompareEvidence.BlockHash()) {
		return errors.New("blocks can not be same")
	}

	if common.BytesToHexString(d.Evidence.Header) >
		common.BytesToHexString(d.CompareEvidence.Header) {
		return errors.New("evidence order error")
	}

	if d.CoinType == payload.ELACoin {
		var err error
		var header, compareHeader *Header
		var confirm, compareConfirm *payload.Confirm

		if header, compareHeader, err = checkDPOSElaIllegalBlockHeaders(d); err != nil {
			return err
		}

		if confirm, compareConfirm, err = checkDPOSElaIllegalBlockConfirms(
			d, header, compareHeader); err != nil {
			return err
		}

		if err := checkDPOSElaIllegalBlockSigners(d, confirm, compareConfirm); err != nil {
			return err
		}
	} else {
		return errors.New("unknown coin type")
	}

	return nil
}

func checkDPOSElaIllegalBlockSigners(
	d *payload.DPOSIllegalBlocks, confirm *payload.Confirm,
	compareConfirm *payload.Confirm) error {

	signers := d.Evidence.Signers
	compareSigners := d.CompareEvidence.Signers

	if len(signers) != len(confirm.Votes) ||
		len(compareSigners) != len(compareConfirm.Votes) {
		return errors.New("signers count it not match the count of " +
			"confirm votes")
	}

	arbitratorsSet := make(map[string]interface{})
	for _, v := range DefaultLedger.Arbitrators.GetArbitrators() {
		arbitratorsSet[common.BytesToHexString(v)] = nil
	}

	for _, v := range signers {
		if _, ok := arbitratorsSet[common.BytesToHexString(v)]; !ok &&
			!DefaultLedger.Arbitrators.IsDisabledProducer(v) {
			return errors.New("invalid signers within evidence")
		}
	}

	for _, v := range compareSigners {
		if _, ok := arbitratorsSet[common.BytesToHexString(v)]; !ok &&
			!DefaultLedger.Arbitrators.IsDisabledProducer(v) {
			return errors.New("invalid signers within evidence")
		}
	}

	confirmSigners := getConfirmSigners(confirm)
	for _, v := range signers {
		if _, ok := confirmSigners[common.BytesToHexString(v)]; !ok {
			return errors.New("signers and confirm votes do not match")
		}
	}

	compareConfirmSigners := getConfirmSigners(compareConfirm)
	for _, v := range compareSigners {
		if _, ok := compareConfirmSigners[common.BytesToHexString(v)]; !ok {
			return errors.New("signers and confirm votes do not match")
		}
	}

	return nil
}

func checkDPOSElaIllegalBlockConfirms(d *payload.DPOSIllegalBlocks,
	header *Header, compareHeader *Header) (*payload.Confirm,
	*payload.Confirm, error) {

	confirm := &payload.Confirm{}
	compareConfirm := &payload.Confirm{}

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

	if err := ConfirmSanityCheck(confirm); err != nil {
		return nil, nil, err
	}
	if err := ConfirmContextCheck(confirm); err != nil {
		return nil, nil, err
	}

	if err := ConfirmSanityCheck(compareConfirm); err != nil {
		return nil, nil, err
	}
	if err := ConfirmContextCheck(compareConfirm); err != nil {
		return nil, nil, err
	}

	if confirm.Proposal.ViewOffset != compareConfirm.Proposal.ViewOffset {
		return nil, nil, errors.New("confirm view offset should not be same")
	}

	if !confirm.Proposal.BlockHash.IsEqual(header.Hash()) {
		return nil, nil, errors.New("block and related confirm do not match")
	}

	if !compareConfirm.Proposal.BlockHash.IsEqual(compareHeader.Hash()) {
		return nil, nil, errors.New("block and related confirm do not match")
	}

	return confirm, compareConfirm, nil
}

func checkDPOSElaIllegalBlockHeaders(d *payload.DPOSIllegalBlocks) (*Header,
	*Header, error) {

	header := &Header{}
	compareHeader := &Header{}

	data := new(bytes.Buffer)
	data.Write(d.Evidence.Header)
	if err := header.Deserialize(data); err != nil {
		return nil, nil, err
	}

	data = new(bytes.Buffer)
	data.Write(d.CompareEvidence.Header)
	if err := compareHeader.Deserialize(data); err != nil {
		return nil, nil, err
	}

	if header.Height != d.BlockHeight || compareHeader.Height != d.BlockHeight {
		return nil, nil, errors.New("block header height should be same")
	}

	//todo check header content later if needed
	// (there is no need to check headers sanity, because arbiters check these
	// headers already. On the other hand, if arbiters do evil to sign multiple
	// headers that are not valid, normal node shall not attach to the chain.
	// So there is no motivation for them to do this.)

	return header, compareHeader, nil
}

func getConfirmSigners(
	confirm *payload.Confirm) map[string]interface{} {
	result := make(map[string]interface{})
	for _, v := range confirm.Votes {
		result[common.BytesToHexString(v.Signer)] = nil
	}
	return result
}

func checkStringField(rawStr string, field string) error {
	if len(rawStr) == 0 || len(rawStr) > MaxStringLength {
		return fmt.Errorf("Field %s has invalid string length.", field)
	}

	return nil
}

func validateProposalEvidence(evidence *payload.ProposalEvidence) error {

	header := &Header{}
	buf := new(bytes.Buffer)
	buf.Write(evidence.BlockHeader)

	if err := header.Deserialize(buf); err != nil {
		return err
	}

	if header.Height != evidence.BlockHeight {
		return errors.New("evidence height and block height should match")
	}

	if !header.Hash().IsEqual(evidence.Proposal.BlockHash) {
		return errors.New("proposal hash and block should match")
	}

	return nil
}

func validateVoteEvidence(evidence *payload.VoteEvidence) error {
	if err := validateProposalEvidence(&evidence.ProposalEvidence); err != nil {
		return err
	}

	if !evidence.Proposal.Hash().IsEqual(evidence.Vote.ProposalHash) {
		return errors.New("vote and proposal should match")
	}

	return nil
}
