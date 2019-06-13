package blockchain

import (
	"errors"
	"math"
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	MaxTimeOffsetSeconds = 2 * 60 * 60
)

type BlockValidateAction struct {
	Name    ValidateFuncName
	Handler func(params ...interface{}) error
}

type Validator struct {
	chain                *BlockChain
	spvService           *spv.Service
	checkSanityFunctions []*BlockValidateAction
}

func NewValidator(chain *BlockChain, spv *spv.Service) *Validator {
	v := &Validator{
		chain:      chain,
		spvService: spv,
	}
	v.RegisterFunc(ValidateFuncNames.CheckHeader, v.checkHeader)
	v.RegisterFunc(ValidateFuncNames.CheckTransactionsCount, v.checkTransactionsCount)
	v.RegisterFunc(ValidateFuncNames.CheckBlockSize, v.checkBlockSize)
	v.RegisterFunc(ValidateFuncNames.CheckCoinBaseTransaction, v.checkCoinBaseTransaction)
	v.RegisterFunc(ValidateFuncNames.CheckTransactionsMerkle, v.checkTransactionsMerkle)
	return v
}

func (v *Validator) RegisterFunc(name ValidateFuncName, function func(params ...interface{}) error) {
	for _, action := range v.checkSanityFunctions {
		if action.Name == name {
			action.Handler = function
			return
		}
	}
	v.checkSanityFunctions = append(v.checkSanityFunctions, &BlockValidateAction{Name: name, Handler: function})
}

func (v *Validator) CheckBlockSanity(block *types.Block, powLimit *big.Int, timeSource MedianTimeSource) error {
	for _, checkFunc := range v.checkSanityFunctions {
		if err := checkFunc.Handler(block, powLimit, timeSource); err != nil {
			return errors.New("[powCheckBlockSanity] error:" + err.Error())
		}
	}
	return nil
}

func (v *Validator) CheckBlockContext(block *types.Block, prevNode *BlockNode) (err error) {
	header := block.Header

	// The genesis block is valid by definition.
	if prevNode == nil {
		return nil
	}

	expectedDifficulty, err := v.chain.CalcNextRequiredDifficulty(
		prevNode, time.Unix(int64(header.GetTimeStamp()), 0))
	if err != nil {
		return err
	}

	if header.GetBits() != expectedDifficulty {
		return errors.New("[powCheckBlockContext] block difficulty is not the expected")
	}

	// Ensure the timestamp for the block header is after the
	// median time of the last several blocks (medianTimeBlocks).
	medianTime := CalcPastMedianTime(prevNode)
	tempTime := time.Unix(int64(header.GetTimeStamp()), 0)

	if !tempTime.After(medianTime) {
		return errors.New("[powCheckBlockContext] block timestamp is not after expected")
	}

	// The height of this block is one more than the referenced
	// previous block.
	blockHeight := prevNode.Height + 1

	// Ensure all transactions in the block are finalized.
	for _, txn := range block.Transactions[1:] {
		if err := CheckTransactionFinalize(txn, blockHeight); err != nil {
			return errors.New("[powCheckBlockContext] block contains unfinalized transaction")
		}
	}

	return nil
}

//block *Block, powLimit *big.Int, timeSource MedianTimeSource
func (v *Validator) checkHeader(params ...interface{}) (err error) {
	block := AssertBlock(params[0])
	powLimit := AssertBigInt(params[1])
	timeSource := AssertMedianTimeSource(params[2])
	header := block.Header
	if header.GetHeight() >= v.chain.chainParams.CheckPowHeaderHeight {
		if err := v.spvService.CheckCRCArbiterSignature(&header.GetAuxPow().SideAuxBlockTx); err != nil {
			return err
		}
	}

	if !header.GetAuxPow().SideAuxPowCheck(header.Hash()) {
		return errors.New("[powCheckHeader] block check proof is failed")
	}
	if v.checkProofOfWork(header, powLimit) != nil {
		return errors.New("[powCheckHeader] block check proof is failed.")
	}

	// A block timestamp must not have a greater precision than one second.
	tempTime := time.Unix(int64(header.GetTimeStamp()), 0)
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
func (v *Validator) checkTransactionsCount(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	// A block must have at least one transaction.
	numTx := len(block.Transactions)
	if numTx == 0 {
		return errors.New("[powCheckTransactionsCount]  block does not contain any transactions")
	}

	// A block must not have more transactions than the max block payload.
	if numTx > types.MaxTxPerBlock {
		return errors.New("[powCheckTransactionsCount]  block contains too many transactions")
	}

	return nil
}

//block *Block
func (v *Validator) checkBlockSize(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	// A block must not exceed the maximum allowed block payload when serialized.
	blockSize := block.GetSize()
	if blockSize > types.MaxBlockSize {
		return errors.New("[powCheckBlockSize] serialized block is too big")
	}

	return nil
}

//block *Block
func (v *Validator) checkCoinBaseTransaction(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	transactions := block.Transactions
	for index, tx := range transactions {
		// The first transaction in a block must be a coinbase.
		if index == 0 {
			if !tx.IsCoinBaseTx() {
				return errors.New("[checkCoinBaseTransaction] first transaction in block is not a coinbase")
			}
			continue
		}

		// A block must not have more than one coinbase.
		if tx.IsCoinBaseTx() {
			return errors.New("[checkCoinBaseTransaction] block contains second coinbase")
		}
	}

	return nil
}

//block *Block
func (v *Validator) checkTransactionsMerkle(params ...interface{}) (err error) {
	block := AssertBlock(params[0])

	txIds := make([]common.Uint256, 0, len(block.Transactions))
	existingTxIds := make(map[common.Uint256]struct{})
	existingTxInputs := make(map[string]struct{})
	existingMainTxs := make(map[common.Uint256]struct{})
	for _, txn := range block.Transactions {
		txId := txn.Hash()
		// Check for duplicate transactions.
		if _, exists := existingTxIds[txId]; exists {
			return errors.New("[CheckTransactionsMerkle] block contains duplicate transaction")
		}
		existingTxIds[txId] = struct{}{}

		// Check for transaction sanity
		if err := v.chain.cfg.CheckTxSanity(txn); err != nil {
			return errors.New("[CheckTransactionsMerkle] failed when verifiy block")
		}

		// Check for duplicate UTXO inputs in a block
		for _, input := range txn.Inputs {
			referKey := input.ReferKey()
			if _, exists := existingTxInputs[referKey]; exists {
				return errors.New("[CheckTransactionsMerkle] block contains duplicate UTXO")
			}
			existingTxInputs[referKey] = struct{}{}
		}

		if txn.IsRechargeToSideChainTx() {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			// Check for duplicate mainchain tx in a block
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			if _, exists := existingMainTxs[*hash]; exists {
				return errors.New("[CheckTransactionsMerkle] block contains duplicate mainchain Tx")
			}
			existingMainTxs[*hash] = struct{}{}
		}

		// Append transaction to list
		txIds = append(txIds, txId)
	}
	calcTransactionsRoot, err := crypto.ComputeRoot(txIds)
	if err != nil {
		return errors.New("[CheckTransactionsMerkle] merkleTree compute failed")
	}
	if !block.Header.GetMerkleRoot().IsEqual(calcTransactionsRoot) {
		return errors.New("[CheckTransactionsMerkle] block merkle root is invalid")
	}

	return nil
}

func (v *Validator) checkProofOfWork(header interfaces.Header, powLimit *big.Int) (err error) {
	// The target difficulty must be larger than zero.
	target := CompactToBig(header.GetBits())
	if target.Sign() <= 0 {
		return errors.New("[checkProofOfWork], block target difficulty is too low.")
	}

	// The target difficulty must be less than the maximum allowed.
	if target.Cmp(powLimit) > 0 {
		return errors.New("[checkProofOfWork], block target difficulty is higher than max of limit.")
	}

	// The block hash must be less than the claimed target.
	hash := header.GetAuxPow().MainBlockHeader.AuxPow.ParBlockHeader.Hash()

	hashNum := HashToBig(&hash)
	if hashNum.Cmp(target) > 0 {
		return errors.New("[checkProofOfWork], block target difficulty is higher than expected difficulty.")
	}

	return nil
}

func CheckTransactionFinalize(tx *types.Transaction, blockHeight uint32) error {
	// Lock time of zero means the transaction is finalized.
	lockTime := tx.LockTime
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
	for _, txIn := range tx.Inputs {
		if txIn.Sequence != math.MaxUint16 {
			return errors.New("[checkFinalizedTransaction] lock time check failed")
		}
	}
	return nil
}
