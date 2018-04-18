package blockchain

import (
	"errors"
	"fmt"
	"math"
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/errors"

	. "github.com/elastos/Elastos.ELA.Utility/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

const (
	MaxTimeOffsetSeconds = 2 * 60 * 60
)

func PowCheckBlockSanity(block *Block, powLimit *big.Int, timeSource MedianTimeSource) error {
	header := block.Header
	hash := header.Hash()
	if !header.AuxPow.Check(&hash, AuxPowChainID) {
		return errors.New("[PowCheckBlockSanity] block check proof is failed")
	}
	if CheckProofOfWork(header, powLimit) != nil {
		return errors.New("[PowCheckBlockSanity] block check proof is failed.")
	}

	// A block timestamp must not have a greater precision than one second.
	tempTime := time.Unix(int64(header.Timestamp), 0)
	if !tempTime.Equal(time.Unix(tempTime.Unix(), 0)) {
		return errors.New("[PowCheckBlockSanity] block timestamp of has a higher precision than one second")
	}

	// Ensure the block time is not too far in the future.
	maxTimestamp := timeSource.AdjustedTime().Add(time.Second * MaxTimeOffsetSeconds)
	if tempTime.After(maxTimestamp) {
		return errors.New("[PowCheckBlockSanity] block timestamp of is too far in the future")
	}

	// A block must have at least one transaction.
	numTx := len(block.Transactions)
	if numTx == 0 {
		return errors.New("[PowCheckBlockSanity]  block does not contain any transactions")
	}

	// A block must not have more transactions than the max block payload.
	if numTx > config.Parameters.MaxTxInBlock {
		return errors.New("[PowCheckBlockSanity]  block contains too many transactions")
	}

	// A block must not exceed the maximum allowed block payload when serialized.
	blockSize := block.GetSize()
	if blockSize > config.Parameters.MaxBlockSize {
		return errors.New("[PowCheckBlockSanity] serialized block is too big")
	}

	// The first transaction in a block must be a coinbase.
	transactions := block.Transactions
	if !transactions[0].IsCoinBaseTx() {
		return errors.New("[PowCheckBlockSanity] first transaction in block is not a coinbase")
	}

	// A block must not have more than one coinbase.
	for _, tx := range transactions[1:] {
		if tx.IsCoinBaseTx() {
			return errors.New("[PowCheckBlockSanity] block contains second coinbase")
		}
	}

	var tharray []Uint256
	for _, txn := range transactions {
		txhash := txn.Hash()
		tharray = append(tharray, txhash)
	}
	calcTransactionsRoot, err := crypto.ComputeRoot(tharray)
	if err != nil {
		return errors.New("[PowCheckBlockSanity] merkleTree compute failed")
	}
	if !header.MerkleRoot.IsEqual(calcTransactionsRoot) {
		return errors.New("[PowCheckBlockSanity] block merkle root is invalid")
	}

	// Check for duplicate transactions.  This check will be fairly quick
	// since the transaction hashes are already cached due to building the
	// merkle tree above.
	existingTxHashes := make(map[Uint256]struct{})
	for _, txn := range transactions {
		txHash := txn.Hash()
		if _, exists := existingTxHashes[txHash]; exists {
			return errors.New("[PowCheckBlockSanity] block contains duplicate transaction")
		}
		existingTxHashes[txHash] = struct{}{}
	}

	for _, txVerify := range transactions {
		if errCode := CheckTransactionSanity(txVerify); errCode != Success {
			return errors.New(fmt.Sprintf("CheckTransactionSanity failed when verifiy block"))
		}
	}

	return nil
}

func PowCheckBlockContext(block *Block, prevNode *BlockNode, ledger *Ledger) error {
	// The genesis block is valid by definition.
	if prevNode == nil {
		return nil
	}

	header := block.Header
	expectedDifficulty, err := CalcNextRequiredDifficulty(prevNode,
		time.Unix(int64(header.Timestamp), 0))
	if err != nil {
		return err
	}

	if header.Bits != expectedDifficulty {
		return errors.New("block difficulty is not the expected")
	}

	// Ensure the timestamp for the block header is after the
	// median time of the last several blocks (medianTimeBlocks).
	medianTime := CalcPastMedianTime(prevNode)
	tempTime := time.Unix(int64(header.Timestamp), 0)

	if !tempTime.After(medianTime) {
		return errors.New("block timestamp is not after expected")
	}

	// The height of this block is one more than the referenced
	// previous block.
	blockHeight := prevNode.Height + 1

	// Ensure all transactions in the block are finalized.
	for _, txn := range block.Transactions[1:] {
		if !IsFinalizedTransaction(txn, blockHeight) {
			return errors.New("block contains unfinalized transaction")
		}
	}

	// for _, txVerify := range block.Transactions {
	// 	if errCode := CheckTransactionContext(txVerify, ledger); errCode != ErrNoError {
	// 		fmt.Println("CheckTransactionContext failed when verifiy block", errCode)
	// 		return errors.New(fmt.Sprintf("CheckTransactionContext failed when verifiy block"))
	// 	}
	// }

	return nil
}

func CheckProofOfWork(bd *Header, powLimit *big.Int) error {
	// The target difficulty must be larger than zero.
	target := CompactToBig(bd.Bits)
	if target.Sign() <= 0 {
		return errors.New("[BlockValidator], block target difficulty is too low.")
	}

	// The target difficulty must be less than the maximum allowed.
	if target.Cmp(powLimit) > 0 {
		return errors.New("[BlockValidator], block target difficulty is higher than max of limit.")
	}

	// The block hash must be less than the claimed target.
	var hash Uint256

	hash = bd.AuxPow.ParBlockHeader.Hash()

	hashNum := HashToBig(&hash)
	if hashNum.Cmp(target) > 0 {
		return errors.New("[BlockValidator], block target difficulty is higher than expected difficulty.")
	}

	return nil
}

func IsFinalizedTransaction(msgTx *Transaction, blockHeight uint32) bool {
	// Lock time of zero means the transaction is finalized.
	lockTime := msgTx.LockTime
	if lockTime == 0 {
		return true
	}

	//FIXME only height
	if lockTime < blockHeight {
		return true
	}

	// At this point, the transaction's lock time hasn't occurred yet, but
	// the transaction might still be finalized if the sequence number
	// for all transaction inputs is maxed out.
	for _, txIn := range msgTx.Inputs {
		if txIn.Sequence != math.MaxUint16 {
			return false
		}
	}
	return true
}
