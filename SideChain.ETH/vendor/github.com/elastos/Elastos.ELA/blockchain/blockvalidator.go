package blockchain

import (
	"errors"
	"math"
	"math/big"
	"time"

	. "github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/config"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA/errors"

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
		return errors.New("[PowCheckBlockSanity] block check aux pow failed")
	}
	if CheckProofOfWork(&header, powLimit) != nil {
		return errors.New("[PowCheckBlockSanity] block check proof of work failed")
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
	if numTx > config.Parameters.MaxTxsInBlock {
		return errors.New("[PowCheckBlockSanity]  block contains too many transactions")
	}

	// A block must not exceed the maximum allowed block payload when serialized.
	blockSize := block.GetSize()
	if blockSize > config.Parameters.MaxBlockSize {
		return errors.New("[PowCheckBlockSanity] serialized block is too big")
	}

	transactions := block.Transactions
	for index, tx := range transactions {
		// The first transaction in a block must be a coinbase.
		if index == 0 {
			if !tx.IsCoinBaseTx() {
				return errors.New("[PowCheckBlockSanity] first transaction in block is not a coinbase")
			}
			continue
		}

		// A block must not have more than one coinbase.
		if tx.IsCoinBaseTx() {
			return errors.New("[PowCheckBlockSanity] block contains second coinbase")
		}
	}

	// Check transaction outputs after a update checkpoint.
	version := uint32(0)
	if block.Height > BlockHeightCheckTxOut {
		version += CheckTxOut
	}

	txIds := make([]Uint256, 0, len(transactions))
	existingTxIds := make(map[Uint256]struct{})
	existingTxInputs := make(map[string]struct{})
	existingSideTxs := make(map[Uint256]struct{})
	for _, txn := range transactions {
		txId := txn.Hash()
		// Check for duplicate transactions.
		if _, exists := existingTxIds[txId]; exists {
			return errors.New("[PowCheckBlockSanity] block contains duplicate transaction")
		}
		existingTxIds[txId] = struct{}{}

		// Check for transaction sanity
		if errCode := CheckTransactionSanity(version, txn); errCode != Success {
			return errors.New("CheckTransactionSanity failed when verifiy block")
		}

		// Check for duplicate UTXO inputs in a block
		for _, input := range txn.Inputs {
			referKey := input.ReferKey()
			if _, exists := existingTxInputs[referKey]; exists {
				return errors.New("[PowCheckBlockSanity] block contains duplicate UTXO")
			}
			existingTxInputs[referKey] = struct{}{}
		}

		if txn.IsWithdrawFromSideChainTx() {
			witPayload := txn.Payload.(*PayloadWithdrawFromSideChain)

			// Check for duplicate sidechain tx in a block
			for _, hash := range witPayload.SideChainTransactionHashes {
				if _, exists := existingSideTxs[hash]; exists {
					return errors.New("[PowCheckBlockSanity] block contains duplicate sidechain Tx")
				}
				existingSideTxs[hash] = struct{}{}
			}
		}

		// Append transaction to list
		txIds = append(txIds, txId)
	}
	calcTransactionsRoot, err := crypto.ComputeRoot(txIds)
	if err != nil {
		return errors.New("[PowCheckBlockSanity] merkleTree compute failed")
	}
	if !header.MerkleRoot.IsEqual(calcTransactionsRoot) {
		return errors.New("[PowCheckBlockSanity] block merkle root is invalid")
	}

	return nil
}

func CheckBlockContext(block *Block) error {
	var rewardInCoinbase = Fixed64(0)
	var totalTxFee = Fixed64(0)

	for index, tx := range block.Transactions {
		if errCode := CheckTransactionContext(tx); errCode != Success {
			return errors.New("CheckTransactionContext failed when verify block")
		}

		if index == 0 {
			// Calculate reward in coinbase
			for _, output := range tx.Outputs {
				rewardInCoinbase += output.Value
			}
			continue
		}
		// Calculate transaction fee
		totalTxFee += GetTxFee(tx, DefaultLedger.Blockchain.AssetID)
	}

	// Reward in coinbase must match inflation 4% per year
	if rewardInCoinbase-totalTxFee != RewardAmountPerBlock {
		return errors.New("reward amount in coinbase not correct")
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

	for _, tx := range block.Transactions[1:] {
		if !IsFinalizedTransaction(tx, block.Height) {
			return errors.New("block contains unfinalized transaction")
		}
	}

	return nil
}

func CheckProofOfWork(header *Header, powLimit *big.Int) error {
	// The target difficulty must be larger than zero.
	target := CompactToBig(header.Bits)
	if target.Sign() <= 0 {
		return errors.New("[BlockValidator], block target difficulty is too low.")
	}

	// The target difficulty must be less than the maximum allowed.
	if target.Cmp(powLimit) > 0 {
		return errors.New("[BlockValidator], block target difficulty is higher than max of limit.")
	}

	// The block hash must be less than the claimed target.
	hash := header.AuxPow.ParBlockHeader.Hash()

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
