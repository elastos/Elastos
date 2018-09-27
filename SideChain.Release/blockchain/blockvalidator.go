package blockchain

import (
	"errors"
	"math"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/config"
	. "github.com/elastos/Elastos.ELA.SideChain/core"
	. "github.com/elastos/Elastos.ELA.SideChain/errors"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	"math/big"
)

type BlockValidateFunctionName string

const (
	PowCheckHeader             = "powcheckheader"
	PowCheckTransactionsCount  = "powchecktransactionscount"
	PowCheckBlockSize          = "powcheckblocksize"
	PowCheckTransactionsFee    = "powchecktransactionsfee"
	PowCheckTransactionsMerkle = "powchecktransactionsmerkle"

	MaxTimeOffsetSeconds = 2 * 60 * 60
)

var BlockValidator *BlockValidate

type BlockValidate struct {
	checkFunctions       map[BlockValidateFunctionName]func(params ...interface{}) error
	checkSanityFunctions map[BlockValidateFunctionName]func(params ...interface{}) error
}

func InitBlockValidator() {
	BlockValidator = &BlockValidate{}
	BlockValidator.Init()
}

func (v *BlockValidate) Init() {
	v.RegisterFunctions(true, PowCheckHeader, v.powCheckHeader)
	v.RegisterFunctions(true, PowCheckTransactionsCount, v.powCheckTransactionsCount)
	v.RegisterFunctions(true, PowCheckBlockSize, v.powCheckBlockSize)
	v.RegisterFunctions(true, PowCheckTransactionsFee, v.powCheckTransactionsFee)
	v.RegisterFunctions(true, PowCheckTransactionsMerkle, v.powCheckTransactionsMerkle)
}

func (v *BlockValidate) RegisterFunctions(isSanity bool, validateName BlockValidateFunctionName, function func(params ...interface{}) error) {
	if isSanity {
		v.checkSanityFunctions[validateName] = function
	} else {
		v.checkFunctions[validateName] = function
	}
}

func (v *BlockValidate) PowCheckBlockSanity(block *Block, powLimit *big.Int, timeSource MedianTimeSource) (err error) {
	defer func() {
		if p := recover(); p != nil {
			str, ok := p.(string)
			if ok {
				err = errors.New("[PowCheckBlockSanity] block validate invalid parameter:" + str)
			} else {
				err = errors.New("[PowCheckBlockSanity] block validate panic")
			}
		}
	}()

	for _, checkFunc := range v.checkSanityFunctions {
		if err := checkFunc(block, powLimit, timeSource); err != nil {
			return errors.New("[powCheckBlockSanity] error:" + err.Error())
		}
	}
	return nil
}

func (v *BlockValidate) PowCheckBlockContext(block *Block, prevNode *BlockNode) (err error) {
	header := block.Header

	// The genesis block is valid by definition.
	if prevNode == nil {
		return nil
	}

	expectedDifficulty, err := CalcNextRequiredDifficulty(prevNode, time.Unix(int64(header.Timestamp), 0))
	if err != nil {
		return err
	}

	if header.Bits != expectedDifficulty {
		return errors.New("[powCheckBlockContext] block difficulty is not the expected")
	}

	// Ensure the timestamp for the block header is after the
	// median time of the last several blocks (medianTimeBlocks).
	medianTime := CalcPastMedianTime(prevNode)
	tempTime := time.Unix(int64(header.Timestamp), 0)

	if !tempTime.After(medianTime) {
		return errors.New("[powCheckBlockContext] block timestamp is not after expected")
	}

	// The height of this block is one more than the referenced
	// previous block.
	blockHeight := prevNode.Height + 1

	// Ensure all transactions in the block are finalized.
	for _, txn := range block.Transactions[1:] {
		if err := v.CheckFinalizedTransaction(txn, blockHeight); err != nil {
			return errors.New("[powCheckBlockContext] block contains unfinalized transaction")
		}
	}

	return nil
}

func (v *BlockValidate) CheckFinalizedTransaction(msgTx *Transaction, blockHeight uint32) (err error) {
	// Lock time of zero means the transaction is finalized.
	lockTime := msgTx.LockTime
	if lockTime == 0 {
		return nil
	}

	//FIXME only height
	if lockTime < blockHeight {
		return nil
	}

	// At this point, the transaction's lock time hasn't occurred yet, but
	// the transaction might still be finalized if the sequence number
	// for all transaction inputs is maxed out.
	for _, txIn := range msgTx.Inputs {
		if txIn.Sequence != math.MaxUint16 {
			return errors.New("[checkFinalizedTransaction] lock time check failed")
		}
	}
	return nil
}

//block *Block, powLimit *big.Int, timeSource MedianTimeSource
func (v *BlockValidate) powCheckHeader(params ...interface{}) (err error) {
	block := AssertBlock(params[0])
	powLimit := AssertBigInt(params[1])
	timeSource := AssertMedianTimeSource(params[2])
	header := block.Header

	// A block's main chain block header must contain in spv module
	//mainChainBlockHash := header.SideAuxPow.MainBlockHeader.Hash()
	//if err := spv.VerifyElaHeader(&mainChainBlockHash); err != nil {
	//	return err
	//}

	if !header.SideAuxPow.SideAuxPowCheck(header.Hash()) {
		return errors.New("[powCheckHeader] block check proof is failed")
	}
	if v.checkProofOfWork(&header, powLimit) != nil {
		return errors.New("[powCheckHeader] block check proof is failed.")
	}

	// A block timestamp must not have a greater precision than one second.
	tempTime := time.Unix(int64(header.Timestamp), 0)
	if !tempTime.Equal(time.Unix(tempTime.Unix(), 0)) {
		return errors.New("[powCheckHeader] block timestamp of has a higher precision than one second")
	}

	// Ensure the block time is not too far in the future.
	maxTimestamp := timeSource.AdjustedTime().Add(time.Second * MaxTimeOffsetSeconds)
	if tempTime.After(maxTimestamp) {
		return errors.New("[powCheckHeader] block timestamp of is too far in the future")
	}

	return nil
}

//block *Block
func (v *BlockValidate) powCheckTransactionsCount(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	// A block must have at least one transaction.
	numTx := len(block.Transactions)
	if numTx == 0 {
		return errors.New("[powCheckTransactionsCount]  block does not contain any transactions")
	}

	// A block must not have more transactions than the max block payload.
	if numTx > config.Parameters.MaxTxInBlock {
		return errors.New("[powCheckTransactionsCount]  block contains too many transactions")
	}

	return nil
}

//block *Block
func (v *BlockValidate) powCheckBlockSize(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	// A block must not exceed the maximum allowed block payload when serialized.
	blockSize := block.GetSize()
	if blockSize > config.Parameters.MaxBlockSize {
		return errors.New("[powCheckBlockSize] serialized block is too big")
	}

	return nil
}

//block *Block
func (v *BlockValidate) powCheckTransactionsFee(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	transactions := block.Transactions
	var rewardInCoinbase = Fixed64(0)
	var totalTxFee = Fixed64(0)
	for index, tx := range transactions {
		// The first transaction in a block must be a coinbase.
		if index == 0 {
			if !tx.IsCoinBaseTx() {
				return errors.New("[powCheckTransactionsFee] first transaction in block is not a coinbase")
			}
			// Calculate reward in coinbase
			for _, output := range tx.Outputs {
				rewardInCoinbase += output.Value
			}
			continue
		}

		// A block must not have more than one coinbase.
		if tx.IsCoinBaseTx() {
			return errors.New("[powCheckTransactionsFee] block contains second coinbase")
		}

		// Calculate transaction fee
		totalTxFee += TxFeeHelper.GetTxFee(tx, DefaultChain.AssetID)
	}

	// Reward in coinbase must match total transaction fee
	if rewardInCoinbase != totalTxFee {
		return errors.New("[powCheckTransactionsFee] reward amount in coinbase not correct")
	}

	return nil
}

//block *Block
func (v *BlockValidate) powCheckTransactionsMerkle(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	txIds := make([]Uint256, 0, len(block.Transactions))
	existingTxIds := make(map[Uint256]struct{})
	existingTxInputs := make(map[string]struct{})
	existingMainTxs := make(map[Uint256]struct{})
	for _, txn := range block.Transactions {
		txId := txn.Hash()
		// Check for duplicate transactions.
		if _, exists := existingTxIds[txId]; exists {
			return errors.New("[powCheckTransactionsMerkle] block contains duplicate transaction")
		}
		existingTxIds[txId] = struct{}{}

		// Check for transaction sanity
		if errCode := TransactionValidator.CheckTransactionSanity(txn); errCode != Success {
			return errors.New("[powCheckTransactionsMerkle] failed when verifiy block")
		}

		// Check for duplicate UTXO inputs in a block
		for _, input := range txn.Inputs {
			referKey := input.ReferKey()
			if _, exists := existingTxInputs[referKey]; exists {
				return errors.New("[powCheckTransactionsMerkle] block contains duplicate UTXO")
			}
			existingTxInputs[referKey] = struct{}{}
		}

		if txn.IsRechargeToSideChainTx() {
			rechargePayload := txn.Payload.(*PayloadRechargeToSideChain)
			// Check for duplicate mainchain tx in a block
			hash, err := rechargePayload.GetMainchainTxHash()
			if err != nil {
				return err
			}
			if _, exists := existingMainTxs[*hash]; exists {
				return errors.New("[powCheckTransactionsMerkle] block contains duplicate mainchain Tx")
			}
			existingMainTxs[*hash] = struct{}{}
		}

		// Append transaction to list
		txIds = append(txIds, txId)
	}
	calcTransactionsRoot, err := crypto.ComputeRoot(txIds)
	if err != nil {
		return errors.New("[powCheckTransactionsMerkle] merkleTree compute failed")
	}
	if !block.Header.MerkleRoot.IsEqual(calcTransactionsRoot) {
		return errors.New("[powCheckTransactionsMerkle] block merkle root is invalid")
	}

	return nil
}

func (v *BlockValidate) checkProofOfWork(header *Header, powLimit *big.Int) (err error) {
	// The target difficulty must be larger than zero.
	target := CompactToBig(header.Bits)
	if target.Sign() <= 0 {
		return errors.New("[checkProofOfWork], block target difficulty is too low.")
	}

	// The target difficulty must be less than the maximum allowed.
	if target.Cmp(powLimit) > 0 {
		return errors.New("[checkProofOfWork], block target difficulty is higher than max of limit.")
	}

	// The block hash must be less than the claimed target.
	hash := header.SideAuxPow.MainBlockHeader.AuxPow.ParBlockHeader.Hash()

	hashNum := HashToBig(&hash)
	if hashNum.Cmp(target) > 0 {
		return errors.New("[checkProofOfWork], block target difficulty is higher than expected difficulty.")
	}

	return nil
}
