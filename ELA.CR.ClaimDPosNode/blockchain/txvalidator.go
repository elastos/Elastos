// Copyright (c) 2017-2020 The Elastos Foundation
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
	"sort"

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
	elaerr "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/vm"
)

const (
	// MaxStringLength is the maximum length of a string field.
	MaxStringLength = 100

	// Category Data length limit not exceeding 4096 characters
	MaxCategoryDataStringLength = 4096

	// MaxBudgetsCount indicates max budgets count of one proposal.
	MaxBudgetsCount = 128

	// ELIPBudgetsCount indicates budgets count of ELIP.
	ELIPBudgetsCount = 2

	// CRCProposalBudgetsPercentage indicates the percentage of the CRC member
	// address balance available for a single proposal budget.
	CRCProposalBudgetsPercentage = 10
)

// CheckTransactionSanity verifies received single transaction
func (b *BlockChain) CheckTransactionSanity(blockHeight uint32,
	txn *Transaction) elaerr.ELAError {
	if err := b.checkTxHeightVersion(txn, blockHeight); err != nil {
		return elaerr.Simple(elaerr.ErrTxHeightVersion, err)
	}

	if err := checkTransactionSize(txn); err != nil {
		log.Warn("[CheckTransactionSize],", err)
		return elaerr.Simple(elaerr.ErrTxSize, err)
	}

	if err := checkTransactionInput(txn); err != nil {
		log.Warn("[CheckTransactionInput],", err)
		return elaerr.Simple(elaerr.ErrTxInvalidInput, err)
	}

	if err := b.checkTransactionOutput(txn, blockHeight); err != nil {
		log.Warn("[CheckTransactionOutput],", err)
		return elaerr.Simple(elaerr.ErrTxInvalidOutput, err)
	}

	if err := checkAssetPrecision(txn); err != nil {
		log.Warn("[CheckAssetPrecesion],", err)
		return elaerr.Simple(elaerr.ErrTxAssetPrecision, err)
	}

	if err := b.checkAttributeProgram(txn, blockHeight); err != nil {
		log.Warn("[CheckAttributeProgram],", err)
		return elaerr.Simple(elaerr.ErrTxAttributeProgram, err)
	}

	if err := checkTransactionPayload(txn); err != nil {
		log.Warn("[CheckTransactionPayload],", err)
		return elaerr.Simple(elaerr.ErrTxPayload, err)
	}

	if err := checkDuplicateSidechainTx(txn); err != nil {
		log.Warn("[CheckDuplicateSidechainTx],", err)
		return elaerr.Simple(elaerr.ErrTxSidechainDuplicate, err)
	}

	return nil
}

// CheckTransactionContext verifies a transaction with history transaction in ledger
func (b *BlockChain) CheckTransactionContext(blockHeight uint32,
	txn *Transaction, proposalsUsedAmount common.Fixed64) (map[*Input]Output, elaerr.ELAError) {
	// check if duplicated with transaction in ledger
	if exist := b.db.IsTxHashDuplicate(txn.Hash()); exist {
		log.Warn("[CheckTransactionContext] duplicate transaction check failed.")
		return nil, elaerr.Simple(elaerr.ErrTxDuplicate, nil)
	}

	if txn.IsCoinBaseTx() {
		return nil, nil
	}

	references, err := b.UTXOCache.GetTxReference(txn)
	if err != nil {
		log.Warn("[CheckTransactionContext] get transaction reference failed")
		return nil, elaerr.Simple(elaerr.ErrTxUnknownReferredTx, nil)
	}

	// check double spent transaction
	if DefaultLedger.IsDoubleSpend(txn) {
		log.Warn("[CheckTransactionContext] IsDoubleSpend check failed")
		return nil, elaerr.Simple(elaerr.ErrTxDoubleSpend, nil)
	}

	if err := checkTransactionUTXOLock(txn, references); err != nil {
		log.Warn("[CheckTransactionUTXOLock],", err)
		return nil, elaerr.Simple(elaerr.ErrTxUTXOLocked, err)
	}

	switch txn.TxType {
	case IllegalProposalEvidence:
		if err := b.checkIllegalProposalsTransaction(txn); err != nil {
			log.Warn("[CheckIllegalProposalsTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case IllegalVoteEvidence:
		if err := b.checkIllegalVotesTransaction(txn); err != nil {
			log.Warn("[CheckIllegalVotesTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case IllegalBlockEvidence:
		if err := b.checkIllegalBlocksTransaction(txn); err != nil {
			log.Warn("[CheckIllegalBlocksTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case IllegalSidechainEvidence:
		if err := b.checkSidechainIllegalEvidenceTransaction(txn); err != nil {
			log.Warn("[CheckSidechainIllegalEvidenceTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case InactiveArbitrators:
		if err := b.checkInactiveArbitratorsTransaction(txn); err != nil {
			log.Warn("[CheckInactiveArbitrators],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case UpdateVersion:
		if err := b.checkUpdateVersionTransaction(txn); err != nil {
			log.Warn("[checkUpdateVersionTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case SideChainPow:
		arbitrator := DefaultLedger.Arbitrators.GetOnDutyCrossChainArbitrator()
		if err := CheckSideChainPowConsensus(txn, arbitrator); err != nil {
			log.Warn("[CheckSideChainPowConsensus],", err)
			return nil, elaerr.Simple(elaerr.ErrTxSidechainPowConsensus, err)
		}
		if txn.IsNewSideChainPowTx() {
			return references, nil
		}

	case RegisterProducer:
		if err := b.checkRegisterProducerTransaction(txn); err != nil {
			log.Warn("[CheckRegisterProducerTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case NextTurnDPOSInfo:
		if err := b.checkNextTurnDPOSInfoTransaction(txn); err != nil {
			log.Warn("[checkNextTurnDPOSInfoTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil
	case CancelProducer:
		if err := b.checkCancelProducerTransaction(txn); err != nil {
			log.Warn("[CheckCancelProducerTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case UpdateProducer:
		if err := b.checkUpdateProducerTransaction(txn); err != nil {
			log.Warn("[CheckUpdateProducerTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case ActivateProducer:
		if err := b.checkActivateProducerTransaction(txn, blockHeight); err != nil {
			log.Warn("[CheckActivateProducerTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}
		return references, nil

	case RegisterCR:
		if err := b.checkRegisterCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[checkRegisterCRTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case UpdateCR:
		if err := b.checkUpdateCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[ checkUpdateCRTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case UnregisterCR:
		if err := b.checkUnRegisterCRTransaction(txn, blockHeight); err != nil {
			log.Warn("[checkUnRegisterCRTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case CRCProposal:
		if err := b.checkCRCProposalTransaction(txn, blockHeight, proposalsUsedAmount); err != nil {
			log.Warn("[checkCRCProposalTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case CRCProposalReview:
		if err := b.checkCRCProposalReviewTransaction(txn,
			blockHeight); err != nil {
			log.Warn("[checkCRCProposalReviewTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case CRCProposalTracking:
		if err := b.checkCRCProposalTrackingTransaction(txn,
			blockHeight); err != nil {
			log.Warn("[checkCRCProposalTrackingTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case CRCProposalWithdraw:
		if err := b.checkCRCProposalWithdrawTransaction(txn, references,
			blockHeight); err != nil {
			log.Warn("[checkCRCProposalWithdrawTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxPayload, err)
		}

	case WithdrawFromSideChain:
		if err := b.checkWithdrawFromSideChainTransaction(txn, references,
			blockHeight); err != nil {
			log.Warn("[CheckWithdrawFromSideChainTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxSidechainDuplicate, err)
		}

	case TransferCrossChainAsset:
		if err := b.checkTransferCrossChainAssetTransaction(txn, references); err != nil {
			log.Warn("[CheckTransferCrossChainAssetTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxInvalidOutput, err)
		}

	case ReturnDepositCoin:
		if err := b.checkReturnDepositCoinTransaction(
			txn, references, b.GetHeight()); err != nil {
			log.Warn("[CheckReturnDepositCoinTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxReturnDeposit, err)
		}

	case ReturnCRDepositCoin:
		if err := b.checkReturnCRDepositCoinTransaction(
			txn, references, b.GetHeight()); err != nil {
			log.Warn("[CheckReturnDepositCoinTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxReturnDeposit, err)
		}

	case CRCAppropriation:
		if err := b.checkCRCAppropriationTransaction(txn, references); err != nil {
			log.Warn("[checkCRCAppropriationTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxAppropriation, err)
		}
		return references, nil

	case CRCProposalRealWithdraw:
		if err := b.checkCRCProposalRealWithdrawTransaction(txn, references); err != nil {
			log.Warn("[checkCRCProposalRealWithdrawTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxRealWithdraw, err)
		}

	case CRAssetsRectify:
		if err := b.checkCRAssetsRectifyTransaction(txn, references); err != nil {
			log.Warn("[checkCRAssetsRectifyTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxAssetsRectify, err)
		}

	case CRDPOSManagement:
		if err := b.checkCRDPOSManagementTransaction(txn); err != nil {
			log.Warn("[checkCRDPOSManagementTransaction],", err)
			return nil, elaerr.Simple(elaerr.ErrTxCRDPOSManagement, err)
		}
	}

	if err := b.checkTransactionFee(txn, references); err != nil {
		log.Warn("[CheckTransactionFee],", err)
		return nil, elaerr.Simple(elaerr.ErrTxBalance, err)
	}

	if err := checkDestructionAddress(references); err != nil {
		log.Warn("[CheckDestructionAddress], ", err)
		return nil, elaerr.Simple(elaerr.ErrTxInvalidInput, err)
	}

	if err := checkTransactionDepositUTXO(txn, references); err != nil {
		log.Warn("[CheckTransactionDepositUTXO],", err)
		return nil, elaerr.Simple(elaerr.ErrTxInvalidInput, err)
	}

	if err := checkTransactionDepositOutpus(b, txn); err != nil {
		log.Warn("[checkTransactionDepositOutpus],", err)
		return nil, elaerr.Simple(elaerr.ErrTxInvalidInput, err)
	}

	if err := checkTransactionSignature(txn, references); err != nil {
		log.Warn("[CheckTransactionSignature],", err)
		return nil, elaerr.Simple(elaerr.ErrTxSignature, err)
	}

	if err := b.checkInvalidUTXO(txn); err != nil {
		log.Warn("[CheckTransactionCoinbaseLock]", err)
		return nil, elaerr.Simple(elaerr.ErrBlockIneffectiveCoinbase, err)
	}

	if txn.Version >= TxVersion09 {
		producers := b.state.GetActiveProducers()
		if blockHeight < b.chainParams.PublicDPOSHeight {
			producers = append(producers, b.state.GetPendingCanceledProducers()...)
		}
		var candidates []*crstate.Candidate
		if b.crCommittee.IsInVotingPeriod(blockHeight) {
			candidates = b.crCommittee.GetCandidates(crstate.Active)
		} else {
			candidates = []*crstate.Candidate{}
		}
		err := b.checkVoteOutputs(blockHeight, txn.Outputs, references,
			getProducerPublicKeysMap(producers), getCRCIDsMap(candidates))
		if err != nil {
			log.Warn("[CheckVoteOutputs]", err)
			return nil, elaerr.Simple(elaerr.ErrTxInvalidOutput, err)
		}
	}

	return references, nil
}

func getProducerPublicKeysMap(producers []*state.Producer) map[string]struct{} {
	pds := make(map[string]struct{})
	for _, p := range producers {
		pds[common.BytesToHexString(p.Info().OwnerPublicKey)] = struct{}{}
	}
	return pds
}

func getCRCIDsMap(crs []*crstate.Candidate) map[common.Uint168]struct{} {
	codes := make(map[common.Uint168]struct{})
	for _, c := range crs {
		codes[c.Info().CID] = struct{}{}
	}
	return codes
}

func (b *BlockChain) checkVoteOutputs(
	blockHeight uint32, outputs []*Output, references map[*Input]Output,
	pds map[string]struct{}, crs map[common.Uint168]struct{}) error {
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
		votePayload, ok := o.Payload.(*outputpayload.VoteOutput)
		if !ok {
			return errors.New("invalid vote output payload")
		}
		for _, content := range votePayload.Contents {
			switch content.VoteType {
			case outputpayload.Delegate:
				err := b.checkVoteProducerContent(
					content, pds, votePayload.Version, o.Value)
				if err != nil {
					return err
				}
			case outputpayload.CRC:
				err := b.checkVoteCRContent(blockHeight,
					content, crs, votePayload.Version, o.Value)
				if err != nil {
					return err
				}
			case outputpayload.CRCProposal:
				err := b.checkVoteCRCProposalContent(
					content, votePayload.Version, o.Value)
				if err != nil {
					return err
				}
			case outputpayload.CRCImpeachment:
				err := b.checkCRImpeachmentContent(
					content, votePayload.Version, o.Value)
				if err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func (b *BlockChain) checkCRImpeachmentContent(content outputpayload.VoteContent,
	payloadVersion byte, amount common.Fixed64) error {
	if payloadVersion < outputpayload.VoteProducerAndCRVersion {
		return errors.New("payload VoteProducerVersion not support vote CRCProposal")
	}

	crMembersMap := getCRMembersMap(b.crCommittee.GetImpeachableMembers())
	for _, cv := range content.CandidateVotes {
		if _, ok := crMembersMap[common.BytesToHexString(cv.Candidate)]; !ok {
			return errors.New("candidate should be one of the CR members")
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

func (b *BlockChain) checkVoteCRContent(blockHeight uint32,
	content outputpayload.VoteContent, crs map[common.Uint168]struct{},
	payloadVersion byte, amount common.Fixed64) error {

	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("cr vote tx must during voting period")
	}

	if payloadVersion < outputpayload.VoteProducerAndCRVersion {
		return errors.New("payload VoteProducerVersion not support vote CR")
	}
	if blockHeight >= b.chainParams.CheckVoteCRCountHeight {
		if len(content.CandidateVotes) > outputpayload.MaxVoteProducersPerTransaction {
			return errors.New("invalid count of CR candidates ")
		}
	}
	for _, cv := range content.CandidateVotes {
		cid, err := common.Uint168FromBytes(cv.Candidate)
		if err != nil {
			return fmt.Errorf("invalid vote output payload " +
				"Candidate can not change to proper cid")
		}
		if _, ok := crs[*cid]; !ok {
			return fmt.Errorf("invalid vote output payload "+
				"CR candidate: %s", cid.String())
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

func (b *BlockChain) checkVoteCRCProposalContent(
	content outputpayload.VoteContent, payloadVersion byte,
	amount common.Fixed64) error {

	if payloadVersion < outputpayload.VoteProducerAndCRVersion {
		return errors.New("payload VoteProducerVersion not support vote CRCProposal")
	}

	for _, cv := range content.CandidateVotes {
		if cv.Votes > amount {
			return errors.New("votes larger than output amount")
		}
		proposalHash, err := common.Uint256FromBytes(cv.Candidate)
		if err != nil {
			return err
		}
		proposal := b.crCommittee.GetProposal(*proposalHash)
		if proposal == nil || proposal.Status != crstate.CRAgreed {
			return fmt.Errorf("invalid CRCProposal: %s",
				common.BytesToHexString(cv.Candidate))
		}
	}

	return nil
}

func getCRMembersMap(members []*crstate.CRMember) map[string]struct{} {
	crMaps := make(map[string]struct{})
	for _, c := range members {
		crMaps[c.Info.CID.String()] = struct{}{}
	}
	return crMaps
}

func checkDestructionAddress(references map[*Input]Output) error {
	for _, output := range references {
		if output.ProgramHash == config.DestroyELAAddress {
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
		txn.IsActivateProducerTx() || txn.IsNextTurnDPOSInfoTx() {
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

func (b *BlockChain) checkCRCProposalWithdrawOutput(txn *Transaction) error {
	withdrawPayload, ok := txn.Payload.(*payload.CRCProposalWithdraw)
	if !ok {
		return errors.New("checkCRCProposalWithdrawOutput invalid payload")
	}
	proposalState := b.crCommittee.GetProposal(withdrawPayload.ProposalHash)
	if proposalState == nil {
		return errors.New("proposal not exist")
	}

	if txn.PayloadVersion == payload.CRCProposalWithdrawDefault {
		//check output[0] must equal with Recipient
		if txn.Outputs[0].ProgramHash != proposalState.Recipient {
			return errors.New("txn.Outputs[0].ProgramHash != Recipient")
		}
		//check output[1] if exist must equal with CRCComitteeAddresss
		if len(txn.Outputs) > 1 {
			if txn.Outputs[1].ProgramHash != b.chainParams.CRExpensesAddress {
				return errors.New("txn.Outputs[1].ProgramHash !=CRCComitteeAddresss")
			}
		}
		if len(txn.Outputs) > 2 {
			return errors.New("CRCProposalWithdraw tx should not have over two output")
		}
	} else if txn.PayloadVersion == payload.CRCProposalWithdrawVersion01 {

		//check output[0] must equal with Recipient
		if withdrawPayload.Recipient != proposalState.Recipient {
			return errors.New("withdrawPayload.Recipient != Recipient")
		}

	}

	return nil
}

func (b *BlockChain) checkTransactionOutput(txn *Transaction,
	blockHeight uint32) error {
	if len(txn.Outputs) > math.MaxUint16 {
		return errors.New("output count should not be greater than 65535(MaxUint16)")
	}

	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("coinbase output is not enough, at least 2")
		}

		if blockHeight >= b.chainParams.CRCommitteeStartHeight {
			if !txn.Outputs[0].ProgramHash.IsEqual(b.chainParams.CRAssetsAddress) {
				return errors.New("first output address should be CRC " +
					"foundation address")
			}
		} else if !txn.Outputs[0].ProgramHash.IsEqual(FoundationAddress) {
			return errors.New("first output address should be foundation " +
				"address")
		}

		foundationReward := txn.Outputs[0].Value
		var totalReward = common.Fixed64(0)
		if blockHeight < b.chainParams.PublicDPOSHeight {
			for _, output := range txn.Outputs {
				if output.AssetID != config.ELAAssetID {
					return errors.New("asset ID in coinbase is invalid")
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
		txn.IsUpdateVersion() || txn.IsActivateProducerTx() || txn.IsNextTurnDPOSInfoTx() {
		if len(txn.Outputs) != 0 {
			return errors.New("no cost transactions should have no output")
		}

		return nil
	}

	if txn.IsCRCAppropriationTx() {
		if len(txn.Outputs) != 2 {
			return errors.New("new CRCAppropriation tx must have two output")
		}
		if !txn.Outputs[0].ProgramHash.IsEqual(b.chainParams.CRExpensesAddress) {
			return errors.New("new CRCAppropriation tx must have the first" +
				"output to CR expenses address")
		}
		if !txn.Outputs[1].ProgramHash.IsEqual(b.chainParams.CRAssetsAddress) {
			return errors.New("new CRCAppropriation tx must have the second" +
				"output to CR assets address")
		}
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

	if txn.IsCRCProposalWithdrawTx() {
		err := b.checkCRCProposalWithdrawOutput(txn)
		if err != nil {
			return err
		}
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
		if programHash.IsEqual(config.CRAssetsAddress) {
			return nil
		}
		if programHash.IsEqual(config.CRCExpensesAddress) {
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

func checkTransactionUTXOLock(txn *Transaction, references map[*Input]Output) error {
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

func checkTransactionDepositUTXO(txn *Transaction, references map[*Input]Output) error {
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
			if txn.IsRegisterProducerTx() || txn.IsRegisterCRTx() ||
				txn.IsReturnDepositCoin() || txn.IsReturnCRDepositCoinTx() {
				continue
			}
			if bc.state.ExistProducerByDepositHash(output.ProgramHash) {
				continue
			}
			if bc.crCommittee.ExistCandidateByDepositHash(
				output.ProgramHash) {
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
	if size <= 0 || size > int(pact.MaxBlockContextSize) {
		return fmt.Errorf("Invalid transaction size: %d bytes", size)
	}

	return nil
}

func checkAssetPrecision(txn *Transaction) error {
	for _, output := range txn.Outputs {
		if !checkAmountPrecise(output.Value, config.ELAPrecision) {
			return errors.New("the precision of asset is incorrect")
		}
	}
	return nil
}

func (b *BlockChain) getTransactionFee(tx *Transaction,
	references map[*Input]Output) common.Fixed64 {
	var outputValue common.Fixed64
	var inputValue common.Fixed64
	for _, output := range tx.Outputs {
		outputValue += output.Value
	}
	for _, output := range references {
		inputValue += output.Value
	}

	return inputValue - outputValue
}

func (b *BlockChain) isSmallThanMinTransactionFee(fee common.Fixed64) bool {
	if fee < b.chainParams.MinTransactionFee {
		return true
	}
	return false
}

func (b *BlockChain) checkTransactionFee(tx *Transaction, references map[*Input]Output) error {
	fee := b.getTransactionFee(tx, references)
	if b.isSmallThanMinTransactionFee(fee) {
		return fmt.Errorf("transaction fee not enough")
	}
	// set Fee and FeePerKB if check has passed
	tx.Fee = fee
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
		ActivateProducer, NextTurnDPOSInfo:
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
	case CRCAppropriation, CRAssetsRectify, CRCProposalRealWithdraw:
		if len(tx.Programs) != 0 {
			return errors.New("CRCAppropriation txs should have no programs")
		}
		if len(tx.Attributes) != 0 {
			return errors.New("CRCAppropriation txs should have no attributes")
		}
		return nil
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
	case CRCProposalWithdraw:
		if len(tx.Programs) != 0 && blockHeight < b.chainParams.CRCProposalWithdrawPayloadV1Height {
			return errors.New("crcproposalwithdraw tx should have no programs")
		}

		if tx.PayloadVersion == payload.CRCProposalWithdrawDefault {
			return nil
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

func checkTransactionSignature(tx *Transaction, references map[*Input]Output) error {
	programHashes, err := GetTxProgramHashes(tx, references)
	if (tx.IsCRCProposalWithdrawTx() && tx.PayloadVersion == payload.CRCProposalWithdrawDefault) ||
		tx.IsCRAssetsRectifyTx() || tx.IsCRCProposalRealWithdrawTx() || tx.IsNextTurnDPOSInfoTx() {
		return nil
	}
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
	case *payload.CRCProposal:
	case *payload.CRCProposalReview:
	case *payload.CRCProposalWithdraw:
	case *payload.CRCProposalTracking:
	case *payload.CRCAppropriation:
	case *payload.CRAssetsRectify:
	case *payload.CRCProposalRealWithdraw:
	case *payload.NextTurnDPOSInfo:
	case *payload.CRDPOSManagement:

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
	case RegisterCR, UpdateCR:
		if blockHeight < b.chainParams.CRVotingStartHeight ||
			(blockHeight < b.chainParams.RegisterCRByDIDHeight &&
				txn.PayloadVersion != payload.CRInfoVersion) {
			return errors.New("not support before CRVotingStartHeight")
		}
	case UnregisterCR, ReturnCRDepositCoin:
		if blockHeight < b.chainParams.CRVotingStartHeight {
			return errors.New("not support before CRVotingStartHeight")
		}
	case CRCProposal, CRCProposalReview, CRCProposalTracking, CRCAppropriation,
		CRCProposalWithdraw:
		if blockHeight < b.chainParams.CRCommitteeStartHeight {
			return errors.New("not support before CRCommitteeStartHeight")
		}
		if txn.TxType == CRCProposalWithdraw {
			if txn.PayloadVersion == payload.CRCProposalWithdrawDefault &&
				blockHeight >= b.chainParams.CRCProposalWithdrawPayloadV1Height {
				return errors.New("not support after CRCProposalWithdrawPayloadV1Height")
			}

			if txn.PayloadVersion == payload.CRCProposalWithdrawVersion01 &&
				blockHeight < b.chainParams.CRCProposalWithdrawPayloadV1Height {
				return errors.New("not support before CRCProposalWithdrawPayloadV1Height")
			}
		}
	case CRAssetsRectify, CRCProposalRealWithdraw:
		if blockHeight < b.chainParams.CRAssetsRectifyTransactionHeight {
			return errors.New("not support before CRAssetsRectifyTransactionHeight")
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

func (b *BlockChain) checkWithdrawFromSideChainTransaction(txn *Transaction, references map[*Input]Output, height uint32) error {
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
		publicKeys, m, n, err := crypto.ParseCrossChainScriptV1(p.Code)
		if err != nil {
			return err
		}

		if height > b.chainParams.CRClaimDPOSNodeStartHeight {
			arbitersCount := len(b.chainParams.CRCArbiters)
			minCount := b.chainParams.CRAgreementCount
			if n != arbitersCount {
				return errors.New("invalid arbiters total count in code")
			}
			if m < int(minCount) {
				return errors.New("invalid arbiters sign count in code")
			}
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

func (b *BlockChain) checkTransferCrossChainAssetTransaction(txn *Transaction, references map[*Input]Output) error {
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

func (b *BlockChain) IsNextArbtratorsSame(nextTurnDPOSInfo *payload.NextTurnDPOSInfo, curNodeNextArbitrators [][]byte) bool {
	if len(nextTurnDPOSInfo.CRPublickeys)+len(nextTurnDPOSInfo.DPOSPublicKeys) != len(curNodeNextArbitrators) {
		log.Warn("IsNextArbtratorsSame curNodeArbitrators len ", len(curNodeNextArbitrators))
		return false
	}
	crindex := 0
	dposIndex := 0
	for _, v := range curNodeNextArbitrators {
		if DefaultLedger.Arbitrators.IsNextCRCArbier(v) {
			if bytes.Equal(v, nextTurnDPOSInfo.CRPublickeys[crindex]) {
				crindex++
				continue
			} else {
				return false
			}
		} else {
			if bytes.Equal(v, nextTurnDPOSInfo.DPOSPublicKeys[dposIndex]) {
				dposIndex++
				continue
			} else {
				return false
			}
		}
	}
	return true
}

func (b *BlockChain) ConvertToArbitersStr(arbiters [][]byte) []string {
	var arbitersStr []string
	for _, v := range arbiters {
		arbitersStr = append(arbitersStr, common.BytesToHexString(v))
	}
	return arbitersStr
}

func (b *BlockChain) checkNextTurnDPOSInfoTransaction(txn *Transaction) error {
	nextTurnDPOSInfo, ok := txn.Payload.(*payload.NextTurnDPOSInfo)
	if !ok {
		return errors.New("invalid NextTurnDPOSInfo payload")
	}
	log.Warnf("[checkNextTurnDPOSInfoTransaction] CRPublickeys %v, DPOSPublicKeys%v\n",
		b.ConvertToArbitersStr(nextTurnDPOSInfo.CRPublickeys), b.ConvertToArbitersStr(nextTurnDPOSInfo.DPOSPublicKeys))

	if !DefaultLedger.Arbitrators.IsNeedNextTurnDPOSInfo() {
		log.Warn("[checkNextTurnDPOSInfoTransaction] !IsNeedNextTurnDPOSInfo")
		return errors.New("should not have next turn dpos info transaction")
	}
	curNodeNextArbitrators := DefaultLedger.Arbitrators.GetNextArbitrators()

	if !b.IsNextArbtratorsSame(nextTurnDPOSInfo, curNodeNextArbitrators) {
		return errors.New("checkNextTurnDPOSInfoTransaction nextTurnDPOSInfo was wrong")
	}
	return nil
}

func (b *BlockChain) checkRegisterProducerTransaction(txn *Transaction) error {
	info, ok := txn.Payload.(*payload.ProducerInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	if err := checkStringField(info.NickName, "NickName", false); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url", true); err != nil {
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

	// check deposit coin
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
			if output.Value < crstate.MinDepositAmount {
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

func (b *BlockChain) checkActivateProducerSignature(activateProducer *payload.ActivateProducer) error {
	// check signature
	publicKey, err := DecodePoint(activateProducer.NodePublicKey)
	if err != nil {
		return errors.New("invalid public key in payload")
	}
	signedBuf := new(bytes.Buffer)
	err = activateProducer.SerializeUnsigned(signedBuf, payload.ActivateProducerVersion)
	if err != nil {
		return err
	}
	err = Verify(*publicKey, signedBuf.Bytes(), activateProducer.Signature)
	if err != nil {
		return errors.New("invalid signature in payload")
	}
	return nil
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

	activateProducer, ok := txn.Payload.(*payload.ActivateProducer)
	if !ok {
		return errors.New("invalid payload")
	}

	err := b.checkActivateProducerSignature(activateProducer)
	if err != nil {
		return err
	}

	if b.GetCRCommittee().IsInElectionPeriod() {
		crMember := b.GetCRCommittee().GetMemberByNodePublicKey(activateProducer.NodePublicKey)
		if crMember != nil && crMember.MemberState == crstate.MemberInactive {
			// todo check penalty
			return nil
		}
	}

	producer := b.state.GetProducer(activateProducer.NodePublicKey)
	if producer == nil || !bytes.Equal(producer.NodePublicKey(),
		activateProducer.NodePublicKey) {
		return errors.New("getting unknown producer")
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

		utxos, err := b.db.GetFFLDB().GetUTXO(programHash)
		if err != nil {
			return err
		}

		for _, u := range utxos {
			depositAmount += u.Value
		}
	} else {
		depositAmount = producer.DepositAmount()
	}

	if depositAmount-producer.Penalty() < crstate.MinDepositAmount {
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
	if err := checkStringField(info.NickName, "NickName", false); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url", true); err != nil {
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

func getDIDFromCode(code []byte) (*common.Uint168, error) {
	newCode := make([]byte, len(code))
	copy(newCode, code)
	didCode := append(newCode[:len(newCode)-1], common.DID)

	if ct1, err := contract.CreateCRIDContractByCode(didCode); err != nil {
		return nil, err
	} else {
		return ct1.ToProgramHash(), nil
	}
}

func (b *BlockChain) checkRegisterCRTransaction(txn *Transaction,
	blockHeight uint32) error {
	info, ok := txn.Payload.(*payload.CRInfo)
	if !ok {
		return errors.New("invalid payload")
	}

	if err := checkStringField(info.NickName, "NickName", false); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url", true); err != nil {
		return err
	}

	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("should create tx during voting period")
	}

	if b.crCommittee.ExistCandidateByNickname(info.NickName) {
		return fmt.Errorf("nick name %s already inuse", info.NickName)
	}

	cr := b.crCommittee.GetCandidate(info.CID)
	if cr != nil {
		return fmt.Errorf("cid %s already exist", info.CID)
	}

	// get CID program hash and check length of code
	ct, err := contract.CreateCRIDContractByCode(info.Code)
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

	// check CID
	if !info.CID.IsEqual(*programHash) {
		return errors.New("invalid cid address")
	}

	if blockHeight >= b.chainParams.RegisterCRByDIDHeight &&
		txn.PayloadVersion == payload.CRInfoDIDVersion {
		// get DID program hash

		programHash, err = getDIDFromCode(info.Code)
		if err != nil {
			return errors.New("invalid info.Code")
		}
		// check DID
		if !info.DID.IsEqual(*programHash) {
			return errors.New("invalid did address")
		}
	}

	// check code and signature
	if err := b.crInfoSanityCheck(info, txn.PayloadVersion); err != nil {
		return err
	}

	// check deposit coin
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
			if output.Value < crstate.MinDepositAmount {
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

	if err := checkStringField(info.NickName, "NickName", false); err != nil {
		return err
	}

	// check url
	if err := checkStringField(info.Url, "Url", true); err != nil {
		return err
	}

	// get CID program hash and check length of code
	ct, err := contract.CreateCRIDContractByCode(info.Code)
	if err != nil {
		return err
	}
	programHash := ct.ToProgramHash()
	if err != nil {
		return err
	}

	// check CID
	if !info.CID.IsEqual(*programHash) {
		return errors.New("invalid cid address")
	}

	if blockHeight >= b.chainParams.RegisterCRByDIDHeight &&
		txn.PayloadVersion == payload.CRInfoDIDVersion {
		// get DID program hash

		programHash, err = getDIDFromCode(info.Code)
		if err != nil {
			return errors.New("invalid info.Code")
		}
		// check DID
		if !info.DID.IsEqual(*programHash) {
			return errors.New("invalid did address")
		}
	}

	// check code and signature
	if err := b.crInfoSanityCheck(info, txn.PayloadVersion); err != nil {
		return err
	}
	if !b.crCommittee.IsInVotingPeriod(blockHeight) {
		return errors.New("should create tx during voting period")
	}

	cr := b.crCommittee.GetCandidate(info.CID)
	if cr == nil {
		return errors.New("updating unknown CR")
	}
	if cr.State() != crstate.Pending && cr.State() != crstate.Active {
		return errors.New("updating canceled or returned CR")
	}

	// check nickname usage.
	if cr.Info().NickName != info.NickName &&
		b.crCommittee.ExistCandidateByNickname(info.NickName) {
		return fmt.Errorf("nick name %s already exist", info.NickName)
	}

	return nil
}

func (b *BlockChain) checkCRCProposalReviewTransaction(txn *Transaction,
	blockHeight uint32) error {
	crcProposalReview, ok := txn.Payload.(*payload.CRCProposalReview)
	if !ok {
		return errors.New("invalid payload")
	}
	// Check if the proposal exist.
	proposalState := b.crCommittee.GetProposal(crcProposalReview.ProposalHash)
	if proposalState == nil {
		return errors.New("proposal not exist")
	}
	if proposalState.Status != crstate.Registered {
		return errors.New("proposal status is not Registered")
	}

	if crcProposalReview.VoteResult < payload.Approve ||
		(crcProposalReview.VoteResult > payload.Abstain) {
		return errors.New("VoteResult should be known")
	}
	crMember := b.crCommittee.GetMember(crcProposalReview.DID)
	if crMember == nil {
		return errors.New("did correspond crMember not exists")
	}
	if crMember.MemberState != crstate.MemberElected {
		return errors.New("should be an elected CR members")
	}
	exist := b.crCommittee.ExistProposal(crcProposalReview.
		ProposalHash)
	if !exist {
		return errors.New("ProposalHash must exist")
	}
	signedBuf := new(bytes.Buffer)
	err := crcProposalReview.SerializeUnsigned(signedBuf, payload.CRCProposalReviewVersion)
	if err != nil {
		return err
	}
	return checkCRTransactionSignature(crcProposalReview.Signature, crMember.Info.Code,
		signedBuf.Bytes())
}

func getCode(publicKey []byte) ([]byte, error) {
	if pk, err := crypto.DecodePoint(publicKey); err != nil {
		return nil, err
	} else {
		if redeemScript, err := contract.CreateStandardRedeemScript(pk); err != nil {
			return nil, err
		} else {
			return redeemScript, nil
		}
	}
}

func getDiDFromPublicKey(publicKey []byte) (*common.Uint168, error) {
	if code, err := getCode(publicKey); err != nil {
		return nil, err
	} else {
		return getDIDFromCode(code)
	}
}

func (b *BlockChain) checkCRCProposalWithdrawTransaction(txn *Transaction,
	references map[*Input]Output, blockHeight uint32) error {

	if txn.PayloadVersion == payload.CRCProposalWithdrawDefault {
		for _, output := range references {
			if output.ProgramHash != b.chainParams.CRExpensesAddress {
				return errors.New("proposal withdrawal transaction for non-crc committee address")
			}
		}
	}

	withdrawPayload, ok := txn.Payload.(*payload.CRCProposalWithdraw)
	if !ok {
		return errors.New("invalid payload")
	}
	// Check if the proposal exist.
	proposalState := b.crCommittee.GetProposal(withdrawPayload.ProposalHash)
	if proposalState == nil {
		return errors.New("proposal not exist")
	}
	if proposalState.Status != crstate.VoterAgreed &&
		proposalState.Status != crstate.Finished &&
		proposalState.Status != crstate.Aborted &&
		proposalState.Status != crstate.Terminated {
		return errors.New("proposal status is not VoterAgreed , " +
			"Finished, Aborted or Terminated")
	}

	if !bytes.Equal(proposalState.ProposalOwner, withdrawPayload.OwnerPublicKey) {
		return errors.New("the OwnerPublicKey is not owner of proposal")
	}
	fee := b.getTransactionFee(txn, references)
	if b.isSmallThanMinTransactionFee(fee) {
		return fmt.Errorf("transaction fee not enough")
	}
	withdrawAmount := b.crCommittee.AvailableWithdrawalAmount(withdrawPayload.ProposalHash)
	if withdrawAmount == 0 {
		return errors.New("no need to withdraw")
	}
	if txn.PayloadVersion == payload.CRCProposalWithdrawDefault {
		//Recipient count + fee must equal to availableWithdrawalAmount
		if txn.Outputs[0].Value+fee != withdrawAmount {
			return errors.New("txn.Outputs[0].Value + fee != withdrawAmout ")
		}
	} else if txn.PayloadVersion == payload.CRCProposalWithdrawVersion01 {
		// Recipient Amount + fee must equal to availableWithdrawalAmount
		if withdrawPayload.Amount+fee != withdrawAmount {
			return errors.New("withdrawPayload.Amount + fee != withdrawAmount ")
		}
		if withdrawPayload.Amount < b.chainParams.RealWithdrawSingleFee {
			return errors.New("withdraw amount is less than RealWithdrawSingleFee")
		}
	}

	signedBuf := new(bytes.Buffer)
	err := withdrawPayload.SerializeUnsigned(signedBuf, txn.PayloadVersion)
	if err != nil {
		return err
	}
	var code []byte
	if code, err = getCode(withdrawPayload.OwnerPublicKey); err != nil {
		return err
	}
	return checkCRTransactionSignature(withdrawPayload.Signature, code, signedBuf.Bytes())
}

func (b *BlockChain) checkCRCProposalTrackingTransaction(txn *Transaction,
	blockHeight uint32) error {
	cptPayload, ok := txn.Payload.(*payload.CRCProposalTracking)
	if !ok {
		return errors.New("invalid payload")
	}

	// Check if proposal exist.
	proposalState := b.crCommittee.GetProposal(cptPayload.ProposalHash)
	if proposalState == nil {
		return errors.New("proposal not exist")
	}
	if proposalState.Status != crstate.VoterAgreed {
		return errors.New("proposal status is not VoterAgreed")
	}
	// Check proposal tracking count.
	if proposalState.TrackingCount >= b.chainParams.MaxProposalTrackingCount {
		return errors.New("reached max tracking count")
	}

	var result error
	switch cptPayload.ProposalTrackingType {
	case payload.Common:
		result = b.checkCRCProposalCommonTracking(cptPayload, proposalState)
	case payload.Progress:
		result = b.checkCRCProposalProgressTracking(cptPayload, proposalState)
	case payload.Rejected:
		result = b.checkCRCProposalRejectedTracking(cptPayload, proposalState)
	case payload.Terminated:
		result = b.checkCRCProposalTerminatedTracking(cptPayload, proposalState)
	case payload.ChangeOwner:
		result = b.checkCRCProposalOwnerTracking(cptPayload, proposalState)
	case payload.Finalized:
		result = b.checkCRCProposalFinalizedTracking(cptPayload, proposalState)
	default:
		result = errors.New("invalid proposal tracking type")
	}
	return result
}

func (b *BlockChain) checkCRCAppropriationTransaction(txn *Transaction,
	references map[*Input]Output) error {
	// Check if current session has appropriated.
	if !b.crCommittee.IsAppropriationNeeded() {
		return errors.New("should have no appropriation transaction")
	}

	// Inputs need to only from CR assets address
	var totalInput common.Fixed64
	for _, output := range references {
		totalInput += output.Value
		if !output.ProgramHash.IsEqual(b.chainParams.CRAssetsAddress) {
			return errors.New("input does not from CR assets address")
		}
	}

	// Inputs amount need equal to outputs amount
	var totalOutput common.Fixed64
	for _, output := range txn.Outputs {
		totalOutput += output.Value
	}
	if totalInput != totalOutput {
		return fmt.Errorf("inputs does not equal to outputs amount, "+
			"inputs:%s outputs:%s", totalInput, totalOutput)
	}

	// Check output amount to CRExpensesAddress:
	// (CRAssetsAddress + CRExpensesAddress)*CRCAppropriatePercentage/100 -
	// CRExpensesAddress + CRCCommitteeUsedAmount
	//
	// Outputs has check in CheckTransactionOutput function:
	// first one to CRCommitteeAddress, second one to CRAssetsAddress
	appropriationAmount := b.crCommittee.AppropriationAmount
	if appropriationAmount != txn.Outputs[0].Value {
		return fmt.Errorf("invalid appropriation amount %s, need to be %s",
			txn.Outputs[0].Value, appropriationAmount)
	}

	return nil
}

func (b *BlockChain) checkCRCProposalRealWithdrawTransaction(txn *Transaction,
	references map[*Input]Output) error {
	crcRealWithdraw, ok := txn.Payload.(*payload.CRCProposalRealWithdraw)
	if !ok {
		return errors.New("invalid payload")
	}
	txsCount := len(crcRealWithdraw.WithdrawTransactionHashes)
	// check WithdrawTransactionHashes count and output count
	if txsCount != len(txn.Outputs) && txsCount != len(txn.Outputs)-1 {
		return errors.New("invalid real withdraw transaction hashes count")
	}

	// if need change, the last output is only allowed to CRExpensesAddress.
	if txsCount != len(txn.Outputs) {
		toProgramHash := txn.Outputs[len(txn.Outputs)-1].ProgramHash
		if !toProgramHash.IsEqual(b.chainParams.CRExpensesAddress) {
			return errors.New(fmt.Sprintf("last output is invalid"))
		}
	}

	// check other outputs, need to match with WithdrawTransactionHashes
	txs := b.crCommittee.GetRealWithdrawTransactions()
	txsMap := make(map[common.Uint256]struct{})
	for i, hash := range crcRealWithdraw.WithdrawTransactionHashes {
		txInfo, ok := txs[hash]
		if !ok {
			return errors.New("invalid withdraw transaction hash")
		}
		output := txn.Outputs[i]
		if !output.ProgramHash.IsEqual(txInfo.Recipient) {
			return errors.New("invalid real withdraw output address")
		}
		if output.Value != txInfo.Amount-b.chainParams.RealWithdrawSingleFee {
			return errors.New(fmt.Sprintf("invalid real withdraw output "+
				"amount:%s, need to be:%s",
				output.Value, txInfo.Amount-b.chainParams.RealWithdrawSingleFee))
		}
		if _, ok := txsMap[hash]; ok {
			return errors.New("duplicated real withdraw transactions hash")
		}
		txsMap[hash] = struct{}{}
	}

	// check transaction fee
	var inputAmount common.Fixed64
	for _, v := range references {
		inputAmount += v.Value
	}
	var outputAmount common.Fixed64
	for _, o := range txn.Outputs {
		outputAmount += o.Value
	}
	if inputAmount-outputAmount != b.chainParams.RealWithdrawSingleFee*common.Fixed64(txsCount) {
		return errors.New(fmt.Sprintf("invalid real withdraw transaction"+
			" fee:%s, need to be:%s, txsCount:%d", inputAmount-outputAmount,
			b.chainParams.RealWithdrawSingleFee*common.Fixed64(txsCount), txsCount))
	}

	return nil
}

func (b *BlockChain) checkCRAssetsRectifyTransaction(txn *Transaction,
	references map[*Input]Output) error {
	// Inputs count should be less than or equal to MaxCRAssetsAddressUTXOCount
	if len(txn.Inputs) > int(b.chainParams.MaxCRAssetsAddressUTXOCount) {
		return errors.New("inputs count should be less than or " +
			"equal to MaxCRAssetsAddressUTXOCount")
	}

	// Inputs count should be greater than or equal to MinCRAssetsAddressUTXOCount
	if len(txn.Inputs) < int(b.chainParams.MinCRAssetsAddressUTXOCount) {
		return errors.New("inputs count should be greater than or " +
			"equal to MinCRAssetsAddressUTXOCount")
	}

	// Inputs need to only from CR assets address
	var totalInput common.Fixed64
	for _, output := range references {
		totalInput += output.Value
		if !output.ProgramHash.IsEqual(b.chainParams.CRAssetsAddress) {
			return errors.New("input does not from CRAssetsAddress")
		}
	}

	// Outputs count should be only one
	if len(txn.Outputs) != 1 {
		return errors.New("outputs count should be only one")
	}

	// Output should translate to CR assets address only
	if !txn.Outputs[0].ProgramHash.IsEqual(b.chainParams.CRAssetsAddress) {
		return errors.New("output does not to CRAssetsAddress")
	}

	// Inputs amount need equal to outputs amount
	totalOutput := txn.Outputs[0].Value
	if totalInput != totalOutput+b.chainParams.RectifyTxFee {
		return fmt.Errorf("inputs minus outputs does not match with %d sela fee , "+
			"inputs:%s outputs:%s", b.chainParams.RectifyTxFee, totalInput, totalOutput)
	}

	return nil
}

func (b *BlockChain) checkCRDPOSManagementTransaction(txn *Transaction) error {
	manager, ok := txn.Payload.(*payload.CRDPOSManagement)
	if !ok {
		return errors.New("invalid payload")
	}
	did := manager.CRCommitteeDID
	crMember := b.crCommittee.GetMember(did)
	if crMember == nil {
		return errors.New("the originator must be members")
	}
	publicKey, err := crypto.DecodePoint(manager.CRManagementPublicKey)
	log.Debugf("operating public key: %s", *publicKey)
	if err != nil {
		return errors.New("invalid operating public key")
	}

	// check duplication of node.
	if b.state.ProducerNodePublicKeyExists(manager.CRManagementPublicKey) {
		return fmt.Errorf("producer already registered")
	}

	err = b.checkCRDPOSManagementSignature(manager, crMember.Info.Code)
	if err != nil {
		return errors.New("CR claim DPOS signature check failed")
	}

	return nil
}

func (b *BlockChain) checkCRDPOSManagementSignature(
	managementPayload *payload.CRDPOSManagement, code []byte) error {
	signBuf := new(bytes.Buffer)
	managementPayload.SerializeUnsigned(signBuf, payload.CRManagementVersion)
	if err := checkCRTransactionSignature(managementPayload.Signature, code,
		signBuf.Bytes()); err != nil {
		return errors.New("CR signature check failed")
	}
	return nil
}

func (b *BlockChain) checkCRCProposalCommonTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	if cptPayload.Stage != 0 {
		return errors.New("stage should assignment zero value")
	}

	// Check signature.
	return b.normalCheckCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalProgressTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	if int(cptPayload.Stage) >= len(pState.Proposal.Budgets) {
		return errors.New("invalid tracking Stage")
	}
	if _, ok := pState.WithdrawableBudgets[cptPayload.Stage]; ok {
		return errors.New("invalid budgets with tracking budget")
	}

	for _, budget := range pState.Proposal.Budgets {
		if cptPayload.Stage == budget.Stage {
			if budget.Type == payload.Imprest ||
				budget.Type == payload.FinalPayment {
				return errors.New("imprest and final payment not allowed to withdraw")
			}
		}
	}

	// Check signature.
	return b.normalCheckCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalRejectedTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	if int(cptPayload.Stage) >= len(pState.Proposal.Budgets) {
		return errors.New("invalid tracking Stage")
	}
	if _, ok := pState.WithdrawableBudgets[cptPayload.Stage]; ok {
		return errors.New("invalid budgets with tracking budget")
	}

	// Check signature.
	return b.normalCheckCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalTerminatedTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	if cptPayload.Stage != 0 {
		return errors.New("stage should assignment zero value")
	}

	// Check signature.
	return b.normalCheckCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalFinalizedTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	var finalStage byte
	for _, budget := range pState.Proposal.Budgets {
		if budget.Type == payload.FinalPayment {
			finalStage = budget.Stage
		}
	}

	if cptPayload.Stage != finalStage {
		return errors.New("cptPayload.Stage is not proposal final stage")
	}

	// Check signature.
	return b.normalCheckCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalOwnerTracking(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check stage of proposal
	if cptPayload.Stage != 0 {
		return errors.New("stage should assignment zero value")
	}

	// Check signature.
	return b.checkCRCProposalTrackingSignature(cptPayload, pState)
}

func (b *BlockChain) checkCRCProposalTrackingSignature(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check signature of proposal owner.
	if !bytes.Equal(pState.ProposalOwner, cptPayload.OwnerPublicKey) {
		return errors.New("the OwnerPublicKey is not owner of proposal")
	}
	signedBuf := new(bytes.Buffer)
	if err := b.checkProposalOwnerSignature(cptPayload,
		cptPayload.OwnerPublicKey, signedBuf); err != nil {
		return err
	}

	// Check other new owner signature.
	if err := b.checkProposalNewOwnerSignature(cptPayload,
		cptPayload.NewOwnerPublicKey, signedBuf); err != nil {
		return err
	}

	// Check secretary general signature。
	return b.checkSecretaryGeneralSignature(cptPayload, pState, signedBuf)
}

func (b *BlockChain) normalCheckCRCProposalTrackingSignature(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState) error {
	// Check new owner public key.
	if len(cptPayload.NewOwnerPublicKey) != 0 {
		return errors.New("the NewOwnerPublicKey need to be empty")
	}

	// Check signature of proposal owner.
	if !bytes.Equal(pState.ProposalOwner, cptPayload.OwnerPublicKey) {
		return errors.New("the OwnerPublicKey is not owner of proposal")
	}
	signedBuf := new(bytes.Buffer)
	if err := b.checkProposalOwnerSignature(cptPayload,
		cptPayload.OwnerPublicKey, signedBuf); err != nil {
		return err
	}

	// Check new owner signature.
	if len(cptPayload.NewOwnerSignature) != 0 {
		return errors.New("the NewOwnerSignature need to be empty")
	}

	// Write new owner signature.
	err := common.WriteVarBytes(signedBuf, cptPayload.NewOwnerSignature)
	if err != nil {
		return errors.New("failed to write NewOwnerSignature")
	}

	// Check secretary general signature。
	return b.checkSecretaryGeneralSignature(cptPayload, pState, signedBuf)
}

func (b *BlockChain) checkProposalOwnerSignature(
	cptPayload *payload.CRCProposalTracking, pubKey []byte,
	signedBuf *bytes.Buffer) error {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return errors.New("invalid proposal owner")
	}
	lContract, err := contract.CreateStandardContract(publicKey)
	if err != nil {
		return errors.New("invalid proposal owner publicKey")
	}
	err = cptPayload.SerializeUnsigned(signedBuf,
		payload.CRCProposalTrackingVersion)
	if err != nil {
		return err
	}
	if err := checkCRTransactionSignature(cptPayload.OwnerSignature, lContract.Code,
		signedBuf.Bytes()); err != nil {
		return errors.New("proposal owner signature check failed")
	}

	return common.WriteVarBytes(signedBuf, cptPayload.OwnerSignature)
}

func (b *BlockChain) checkProposalNewOwnerSignature(
	cptPayload *payload.CRCProposalTracking, pubKey []byte,
	signedBuf *bytes.Buffer) error {
	publicKey, err := crypto.DecodePoint(pubKey)
	if err != nil {
		return errors.New("invalid new proposal owner public key")
	}
	lContract, err := contract.CreateStandardContract(publicKey)
	if err != nil {
		return errors.New("invalid new proposal owner publicKey")
	}
	if err := checkCRTransactionSignature(cptPayload.NewOwnerSignature, lContract.Code,
		signedBuf.Bytes()); err != nil {
		return errors.New("new proposal owner signature check failed")
	}

	return common.WriteVarBytes(signedBuf, cptPayload.NewOwnerSignature)
}

func (b *BlockChain) checkSecretaryGeneralSignature(
	cptPayload *payload.CRCProposalTracking, pState *crstate.ProposalState,
	signedBuf *bytes.Buffer) error {
	var sgContract *contract.Contract
	publicKeyBytes, err := hex.DecodeString(b.crCommittee.GetProposalManager().SecretaryGeneralPublicKey)
	if err != nil {
		return errors.New("invalid secretary general public key")
	}
	publicKey, err := crypto.DecodePoint(publicKeyBytes)
	if err != nil {
		return errors.New("invalid proposal secretary general public key")
	}
	sgContract, err = contract.CreateStandardContract(publicKey)
	if err != nil {
		return errors.New("invalid secretary general public key")
	}
	if _, err := signedBuf.Write([]byte{byte(cptPayload.ProposalTrackingType)}); err != nil {
		return errors.New("invalid ProposalTrackingType")
	}
	if err := cptPayload.SecretaryGeneralOpinionHash.Serialize(signedBuf); err != nil {
		return errors.New("invalid secretary opinion hash")
	}
	if err = checkCRTransactionSignature(cptPayload.SecretaryGeneralSignature,
		sgContract.Code, signedBuf.Bytes()); err != nil {
		return errors.New("secretary general signature check failed")
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

	cr := b.crCommittee.GetCandidate(info.CID)
	if cr == nil {
		return errors.New("unregister unknown CR")
	}
	if cr.State() != crstate.Pending && cr.State() != crstate.Active {
		return errors.New("unregister canceled or returned CR")
	}

	signedBuf := new(bytes.Buffer)
	err := info.SerializeUnsigned(signedBuf, payload.UnregisterCRVersion)
	if err != nil {
		return err
	}
	return checkCRTransactionSignature(info.Signature, cr.Info().Code, signedBuf.Bytes())
}

func (b *BlockChain) isPublicKeyDIDMatch(pubKey []byte, did *common.Uint168) bool {
	var code []byte
	var err error
	//get Code
	if code, err = getCode(pubKey); err != nil {
		return false
	}
	//get DID
	var didGenerated *common.Uint168
	if didGenerated, err = getDIDFromCode(code); err != nil {
		return false
	}
	if !did.IsEqual(*didGenerated) {
		return false
	}
	return true
}

func (b *BlockChain) checkProposalOwnerSign(crcProposal *payload.CRCProposal, signedBuf *bytes.Buffer) error {
	//get ownerCode
	var code []byte
	var err error
	if code, err = getCode(crcProposal.OwnerPublicKey); err != nil {
		return err
	}
	// get verify data
	err = crcProposal.SerializeUnsigned(signedBuf, payload.CRCProposalVersion)
	if err != nil {
		return err
	}
	//verify sign
	if err := checkCRTransactionSignature(crcProposal.Signature, code,
		signedBuf.Bytes()); err != nil {
		return errors.New("owner signature check failed")
	}
	return nil
}

func (b *BlockChain) checkSecretaryGeneralSign(crcProposal *payload.CRCProposal, signedBuf *bytes.Buffer) error {
	var code []byte
	var err error
	if code, err = getCode(crcProposal.SecretaryGeneralPublicKey); err != nil {
		return err
	}
	if err = checkCRTransactionSignature(crcProposal.SecretaryGeneraSignature, code,
		signedBuf.Bytes()); err != nil {
		return errors.New("failed to check SecretaryGeneral signature")
	}
	return nil
}

func (b *BlockChain) checkProposalCRCouncilMemberSign(crcProposal *payload.CRCProposal, code []byte,
	signedBuf *bytes.Buffer) error {

	// Check signature of CR Council Member.
	if err := common.WriteVarBytes(signedBuf, crcProposal.Signature); err != nil {
		return errors.New("failed to write proposal owner signature")
	}
	if err := common.WriteVarBytes(signedBuf, crcProposal.SecretaryGeneraSignature); err != nil {
		return errors.New("failed to write SecretaryGenera Signature")
	}
	if err := crcProposal.CRCouncilMemberDID.Serialize(signedBuf); err != nil {
		return errors.New("failed to write CR Council Member's DID")
	}
	if err := checkCRTransactionSignature(crcProposal.CRCouncilMemberSignature, code,
		signedBuf.Bytes()); err != nil {
		return errors.New("failed to check CR Council Member signature")
	}

	return nil
}
func (b *BlockChain) checkChangeSecretaryGeneralProposalTx(crcProposal *payload.CRCProposal) error {
	// The number of the proposals of the committee can not more than 128
	if !b.isPublicKeyDIDMatch(crcProposal.SecretaryGeneralPublicKey, &crcProposal.SecretaryGeneralDID) {
		return errors.New("SecretaryGeneral PublicKey and DID is not matching")
	}
	// Check owner public key
	if _, err := crypto.DecodePoint(crcProposal.OwnerPublicKey); err != nil {
		return errors.New("invalid owner public key")
	}

	//CRCouncilMemberDID must MemberElected cr member
	if !b.crCommittee.IsElectedCRMemberByDID(crcProposal.CRCouncilMemberDID) {
		return errors.New("CR Council Member should be one elected CR members")
	}
	//verify 3 signature(owner signature , new secretary general, CRCouncilMember)

	//Check signature of owner
	signedBuf := new(bytes.Buffer)
	if err := b.checkProposalOwnerSign(crcProposal, signedBuf); err != nil {
		return errors.New("owner signature check failed")
	}
	// Check signature of SecretaryGeneral
	if err := b.checkSecretaryGeneralSign(crcProposal, signedBuf); err != nil {
		return errors.New("SecretaryGeneral signature check failed")
	}
	// Check signature of CR Council Member.
	crMember := b.crCommittee.GetMember(crcProposal.CRCouncilMemberDID)
	if crMember == nil {
		return errors.New("CR Council Member should be one of the CR members")
	}
	if err := b.checkProposalCRCouncilMemberSign(crcProposal, crMember.Info.Code, signedBuf); err != nil {
		return errors.New("CR Council Member signature check failed")
	}
	return nil
}

func (b *BlockChain) checkCloseProposal(proposal *payload.CRCProposal) error {
	_, err := crypto.DecodePoint(proposal.OwnerPublicKey)
	if err != nil {
		return errors.New("DecodePoint from OwnerPublicKey error")
	}
	if ps := b.crCommittee.GetProposal(proposal.TargetProposalHash); ps == nil {
		return errors.New("CloseProposalHash does not exist")
	} else if ps.Status != crstate.VoterAgreed {
		return errors.New("CloseProposalHash has to be voterAgreed")
	}
	if len(proposal.Budgets) > 0 {
		return errors.New("CloseProposal cannot have budget")
	}
	emptyUint168 := common.Uint168{}
	if proposal.Recipient != emptyUint168 {
		return errors.New("CloseProposal recipient must be empty")
	}
	crMember := b.crCommittee.GetMember(proposal.CRCouncilMemberDID)
	if crMember == nil {
		return errors.New("CR Council Member should be one of the CR members")
	}
	return b.checkOwnerAndCRCouncilMemberSign(proposal, crMember.Info.Code)
}

func (b *BlockChain) checkChangeProposalOwner(proposal *payload.CRCProposal) error {
	proposalState := b.crCommittee.GetProposal(proposal.TargetProposalHash)
	if proposalState == nil {
		return errors.New("proposal doesn't exist")
	}
	if proposalState.Status != crstate.VoterAgreed {
		return errors.New("proposal status is not VoterAgreed")
	}

	if _, err := crypto.DecodePoint(proposal.OwnerPublicKey); err != nil {
		return errors.New("invalid owner public key")
	}

	if _, err := crypto.DecodePoint(proposal.NewOwnerPublicKey); err != nil {
		return errors.New("invalid new owner public key")
	}

	if bytes.Equal(proposal.NewOwnerPublicKey, proposalState.ProposalOwner) &&
		proposal.NewRecipient.IsEqual(proposalState.Recipient) {
		return errors.New("new owner or recipient must be different from the previous one")
	}

	crCouncilMember := b.crCommittee.GetMember(proposal.CRCouncilMemberDID)
	if crCouncilMember == nil {
		return errors.New("CR Council Member should be one of the CR members")
	}
	return b.checkChangeOwnerSign(proposal, crCouncilMember.Info.Code)
}

func (b *BlockChain) checkChangeOwnerSign(proposal *payload.CRCProposal, crMemberCode []byte) error {
	signedBuf := new(bytes.Buffer)
	err := proposal.SerializeUnsigned(signedBuf, payload.CRCProposalVersion)
	if err != nil {
		return err
	}
	// Check signature of new owner.
	newOwnerPublicKey, err := crypto.DecodePoint(proposal.NewOwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner")
	}
	newOwnerContract, err := contract.CreateStandardContract(newOwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner")
	}

	if err := checkCRTransactionSignature(proposal.NewOwnerSignature, newOwnerContract.Code,
		signedBuf.Bytes()); err != nil {
		return errors.New("new owner signature check failed")
	}

	// Check signature of owner.
	publicKey, err := crypto.DecodePoint(proposal.OwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner")
	}
	ownerContract, err := contract.CreateStandardContract(publicKey)
	if err != nil {
		return errors.New("invalid owner")
	}
	if err := checkCRTransactionSignature(proposal.Signature, ownerContract.Code,
		signedBuf.Bytes()); err != nil {
		return errors.New("owner signature check failed")
	}

	// Check signature of CR Council Member.
	if err = common.WriteVarBytes(signedBuf, proposal.NewOwnerSignature); err != nil {
		return errors.New("failed to write proposal new owner signature")
	}
	if err = common.WriteVarBytes(signedBuf, proposal.Signature); err != nil {
		return errors.New("failed to write proposal owner signature")
	}
	if err = proposal.CRCouncilMemberDID.Serialize(signedBuf); err != nil {
		return errors.New("failed to write CR Council Member's DID")
	}
	if err = checkCRTransactionSignature(proposal.CRCouncilMemberSignature, crMemberCode,
		signedBuf.Bytes()); err != nil {
		return errors.New("failed to check CR Council Member signature")
	}

	return nil
}

func (b *BlockChain) checkOwnerAndCRCouncilMemberSign(proposal *payload.CRCProposal, crMemberCode []byte) error {
	// Check signature of owner.
	publicKey, err := crypto.DecodePoint(proposal.OwnerPublicKey)
	if err != nil {
		return errors.New("invalid owner")
	}
	contract, err := contract.CreateStandardContract(publicKey)
	if err != nil {
		return errors.New("invalid owner")
	}
	signedBuf := new(bytes.Buffer)
	err = proposal.SerializeUnsigned(signedBuf, payload.CRCProposalVersion)
	if err != nil {
		return err
	}
	if err := checkCRTransactionSignature(proposal.Signature, contract.Code,
		signedBuf.Bytes()); err != nil {
		return errors.New("owner signature check failed")
	}

	// Check signature of CR Council Member.
	if err = common.WriteVarBytes(signedBuf, proposal.Signature); err != nil {
		return errors.New("failed to write proposal owner signature")
	}
	if err = proposal.CRCouncilMemberDID.Serialize(signedBuf); err != nil {
		return errors.New("failed to write CR Council Member's DID")
	}
	if err = checkCRTransactionSignature(proposal.CRCouncilMemberSignature, crMemberCode,
		signedBuf.Bytes()); err != nil {
		return errors.New("failed to check CR Council Member signature")
	}
	return nil
}

func (b *BlockChain) checkNormalOrELIPProposal(proposal *payload.CRCProposal, proposalsUsedAmount common.Fixed64) error {
	if proposal.ProposalType == payload.ELIP {
		if len(proposal.Budgets) != ELIPBudgetsCount {
			return errors.New("ELIP needs to have and only have two budget")
		}
		for _, budget := range proposal.Budgets {
			if budget.Type == payload.NormalPayment {
				return errors.New("ELIP needs to have no normal payment")
			}
		}
	}
	// Check budgets of proposal
	if len(proposal.Budgets) < 1 {
		return errors.New("a proposal cannot be without a Budget")
	}
	budgets := make([]payload.Budget, len(proposal.Budgets))
	for i, budget := range proposal.Budgets {
		budgets[i] = budget
	}
	sort.Slice(budgets, func(i, j int) bool {
		return budgets[i].Stage < budgets[j].Stage
	})
	if budgets[0].Type == payload.Imprest && budgets[0].Stage != 0 {
		return errors.New("proposal imprest can only be in the first phase")
	}
	if budgets[0].Type != payload.Imprest && budgets[0].Stage != 1 {
		return errors.New("the first general type budget needs to start at the beginning")
	}
	if budgets[len(budgets)-1].Type != payload.FinalPayment {
		return errors.New("proposal final payment can only be in the last phase")
	}
	stage := budgets[0].Stage
	var amount common.Fixed64
	var imprestPaymentCount int
	var finalPaymentCount int
	for _, b := range budgets {
		switch b.Type {
		case payload.Imprest:
			imprestPaymentCount++
		case payload.NormalPayment:
		case payload.FinalPayment:
			finalPaymentCount++
		default:
			return errors.New("type of budget should be known")
		}
		if b.Stage != stage {
			return errors.New("the first phase starts incrementing")
		}
		if b.Amount < 0 {
			return errors.New("invalid amount")
		}
		stage++
		amount += b.Amount
	}
	if imprestPaymentCount > 1 {
		return errors.New("imprest payment count invalid")
	}
	if finalPaymentCount != 1 {
		return errors.New("final payment count invalid")
	}
	if amount > b.crCommittee.CRCCurrentStageAmount*CRCProposalBudgetsPercentage/100 {
		return errors.New("budgets exceeds 10% of CRC committee balance")
	} else if amount > b.crCommittee.CRCCurrentStageAmount-
		b.crCommittee.CRCCommitteeUsedAmount-proposalsUsedAmount {
		return errors.New(fmt.Sprintf("budgets exceeds the balance of CRC"+
			" committee, proposal hash:%s, budgets:%s, need <= %s",
			proposal.Hash(), amount, b.crCommittee.CRCCurrentStageAmount-
				b.crCommittee.CRCCommitteeUsedAmount-proposalsUsedAmount))
	} else if amount < 0 {
		return errors.New("budgets is invalid")
	}
	emptyUint168 := common.Uint168{}
	if proposal.Recipient == emptyUint168 {
		return errors.New("recipient is empty")
	}
	prefix := contract.GetPrefixType(proposal.Recipient)
	if prefix != contract.PrefixStandard && prefix != contract.PrefixMultiSig {
		return errors.New("invalid recipient prefix")
	}
	_, err := proposal.Recipient.ToAddress()
	if err != nil {
		return errors.New("invalid recipient")
	}
	crCouncilMember := b.crCommittee.GetMember(proposal.CRCouncilMemberDID)
	return b.checkOwnerAndCRCouncilMemberSign(proposal, crCouncilMember.Info.Code)
}

func (b *BlockChain) checkCRCProposalTransaction(txn *Transaction,
	blockHeight uint32, proposalsUsedAmount common.Fixed64) error {
	proposal, ok := txn.Payload.(*payload.CRCProposal)
	if !ok {
		return errors.New("invalid payload")
	}
	// The number of the proposals of the committee can not more than 128
	if b.crCommittee.IsProposalFull(proposal.CRCouncilMemberDID) {
		return errors.New("proposal is full")
	}
	// Check draft hash of proposal.
	if b.crCommittee.ExistDraft(proposal.DraftHash) {
		return errors.New("duplicated draft proposal hash")
	}

	if !b.crCommittee.IsProposalAllowed(blockHeight - 1) {
		return errors.New("cr proposal tx must not during voting period")
	}
	if len(proposal.CategoryData) > MaxCategoryDataStringLength {
		return errors.New("the Proposal category data cannot be more than 4096 characters")
	}
	if len(proposal.Budgets) > MaxBudgetsCount {
		return errors.New("budgets exceeded the maximum limit")
	}
	// Check type of proposal.
	if proposal.ProposalType.Name() == "Unknown" {
		return errors.New("type of proposal should be known")
	}
	//CRCouncilMemberDID must MemberElected cr member
	// Check CR Council Member DID of proposal.
	crMember := b.crCommittee.GetMember(proposal.CRCouncilMemberDID)
	if crMember == nil {
		return errors.New("CR Council Member should be one of the CR members")
	}
	if crMember.MemberState != crstate.MemberElected {
		return errors.New("CR Council Member should be an elected CR members")
	}
	switch proposal.ProposalType {
	case payload.ChangeProposalOwner:
		return b.checkChangeProposalOwner(proposal)
	case payload.CloseProposal:
		return b.checkCloseProposal(proposal)
	case payload.SecretaryGeneral:
		return b.checkChangeSecretaryGeneralProposalTx(proposal)
	default:
		return b.checkNormalOrELIPProposal(proposal, proposalsUsedAmount)
	}
}

func getDIDByCode(code []byte) (*common.Uint168, error) {
	didCode := make([]byte, len(code))
	copy(didCode, code)
	didCode = append(didCode[:len(code)-1], common.DID)
	ct1, err := contract.CreateCRIDContractByCode(didCode)
	if err != nil {
		return nil, err
	}
	return ct1.ToProgramHash(), err
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

func (b *BlockChain) crInfoSanityCheck(info *payload.CRInfo, payloadVersion byte) error {
	signedBuf := new(bytes.Buffer)
	err := info.SerializeUnsigned(signedBuf, payloadVersion)
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
	references map[*Input]Output, currentHeight uint32) error {

	var inputValue common.Fixed64
	fromAddrMap := make(map[common.Uint168]struct{})
	for _, output := range references {
		inputValue += output.Value
		fromAddrMap[output.ProgramHash] = struct{}{}
	}

	if len(fromAddrMap) != 1 {
		return errors.New("UTXO should from same deposit address")
	}

	var programHash common.Uint168
	for k := range fromAddrMap {
		programHash = k
	}

	var changeValue common.Fixed64
	var outputValue common.Fixed64
	for _, output := range txn.Outputs {
		if output.ProgramHash.IsEqual(programHash) {
			changeValue += output.Value
		} else {
			outputValue += output.Value
		}
	}

	var depositAmount common.Fixed64
	var penalty common.Fixed64
	for _, program := range txn.Programs {
		p := b.state.GetProducer(program.Code[1 : len(program.Code)-1])
		if p == nil {
			return errors.New("signer must be producer")
		}
		penalty += p.Penalty()
		depositAmount += p.DepositAmount()
	}

	if inputValue-changeValue > depositAmount-penalty ||
		outputValue >= depositAmount {
		return fmt.Errorf("overspend deposit")
	}

	return nil
}

func (b *BlockChain) checkReturnCRDepositCoinTransaction(txn *Transaction,
	references map[*Input]Output, currentHeight uint32) error {

	var inputValue common.Fixed64
	fromAddrMap := make(map[common.Uint168]struct{})
	for _, output := range references {
		inputValue += output.Value
		fromAddrMap[output.ProgramHash] = struct{}{}
	}

	if len(fromAddrMap) != 1 {
		return errors.New("UTXO should from same deposit address")
	}

	var programHash common.Uint168
	for k := range fromAddrMap {
		programHash = k
	}

	var changeValue common.Fixed64
	var outputValue common.Fixed64
	for _, output := range txn.Outputs {
		if output.ProgramHash.IsEqual(programHash) {
			changeValue += output.Value
		} else {
			outputValue += output.Value
		}
	}

	var availableValue common.Fixed64
	for _, program := range txn.Programs {
		// Get candidate from code.
		ct, err := contract.CreateCRIDContractByCode(program.Code)
		if err != nil {
			return err
		}
		cid := ct.ToProgramHash()
		if !b.crCommittee.Exist(*cid) {
			return errors.New("signer must be candidate or member")
		}

		availableValue += b.crCommittee.GetAvailableDepositAmount(*cid)
	}

	// Check output amount.
	if inputValue-changeValue > availableValue ||
		outputValue >= availableValue {
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

	crcArbitratorsCount := DefaultLedger.Arbitrators.GetCRCArbitersCount()
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
		if !DefaultLedger.Arbitrators.IsCRCArbitrator(pk[1:]) {
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

func checkStringField(rawStr string, field string, allowEmpty bool) error {
	if (!allowEmpty && len(rawStr) == 0) || len(rawStr) > MaxStringLength {
		return fmt.Errorf("field %s has invalid string length", field)
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
