package db

import (
	"bytes"
	"errors"
	"math/big"
	"sync"

	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
)

type ChainState int

const (
	SYNCING = ChainState(0)
	WAITING = ChainState(1)
)

const (
	MaxBlockLocatorHashes = 200
)

var PowLimit = new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1))

type Blockchain struct {
	sync.RWMutex
	state ChainState
	Headers
	DataStore
}

func NewBlockchain() (*Blockchain, error) {
	headersDB, err := NewHeadersDB()
	if err != nil {
		return nil, err
	}

	sqliteDb, err := NewSQLiteDB()
	if err != nil {
		return nil, err
	}

	return &Blockchain{
		state:     WAITING,
		Headers:   headersDB,
		DataStore: sqliteDb,
	}, nil
}

func (bc *Blockchain) Close() {
	bc.Lock()
	bc.Headers.Close()
	bc.DataStore.Close()
}

func (bc *Blockchain) SetChainState(state ChainState) {
	bc.Lock()
	defer bc.Unlock()

	bc.state = state
}

func (bc *Blockchain) IsSyncing() bool {
	bc.RLock()
	defer bc.RUnlock()

	return bc.state == SYNCING
}

func (bc *Blockchain) Height() uint32 {
	bc.RLock()
	defer bc.RUnlock()

	tip, err := bc.Headers.GetTip()
	if err != nil {
		return 0
	}

	return tip.Height
}

func (bc *Blockchain) ChainTip() *Header {
	bc.RLock()
	defer bc.RUnlock()

	tip, err := bc.Headers.GetTip()
	if err != nil { // Empty blockchain, return empty header
		return new(Header)
	}
	return tip
}

func (bc *Blockchain) IsKnownBlock(hash Uint256) bool {
	bc.RLock()
	defer bc.RUnlock()

	header, err := bc.Headers.GetHeader(&hash)
	if header == nil || err != nil {
		return false
	}
	return true
}

func (bc *Blockchain) GetBlockLocatorHashes() []*Uint256 {
	bc.RLock()
	defer bc.RUnlock()

	var ret []*Uint256
	parent, err := bc.Headers.GetTip()
	if err != nil { // No headers stored return empty locator
		return ret
	}

	rollback := func(parent *Header, n int) (*Header, error) {
		for i := 0; i < n; i++ {
			parent, err = bc.Headers.GetPrevious(parent)
			if err != nil {
				return parent, err
			}
		}
		return parent, nil
	}

	step := 1
	start := 0
	for {
		if start >= 9 {
			step *= 2
			start = 0
		}
		hash := parent.Hash()
		ret = append(ret, hash)
		if len(ret) >= MaxBlockLocatorHashes {
			break
		}
		parent, err = rollback(parent, step)
		if err != nil {
			break
		}
		start += 1
	}
	return ret
}

func (bc *Blockchain) CommitUnconfirmedTxn(txn tx.Transaction) error {
	bc.Lock()
	defer bc.Unlock()

	return bc.commitTxn(bc.Scripts().GetFilter(), 0, txn)
}

func (bc *Blockchain) CommitBlock(header Header, txns []tx.Transaction) error {
	bc.Lock()
	defer bc.Unlock()

	// Get current chain tip
	tip, err := bc.Headers.GetTip()
	if err != nil {
		return err
	}

	// Check if commit block is a reorganize block, if so rollback to the fork point
	if header.Height < tip.Height && bc.IsKnownBlock(header.Previous) {
		err = bc.rollbackTo(header.Previous)
		if err != nil {
			return err
		}
	}

	// Save transactions first
	err = bc.commitTxns(header, txns)
	if err != nil {
		return err
	}

	return bc.Headers.Add(&header)
}

func (bc *Blockchain) commitTxns(header Header, txns []tx.Transaction) error {
	for _, txn := range txns {
		err := bc.commitTxn(bc.Scripts().GetFilter(), header.Height, txn)
		if err != nil {
			return err
		}
	}

	return nil
}

func (bc *Blockchain) commitTxn(filter *ScriptFilter, height uint32, txn tx.Transaction) error {
	txId := txn.Hash()
	// Save UTXOs
	for index, output := range txn.Outputs {
		if filter.ContainAddress(output.ProgramHash) {
			if txn.TxType == tx.CoinBase {
				output.OutputLock = height + 100
			}
			utxo := ToStordUTXO(txId, height, index, output)
			err := bc.UTXOs().Put(&output.ProgramHash, utxo)
			if err != nil {
				return err
			}
		}
	}

	// Put spent UTXOs to STXOs
	for _, input := range txn.Inputs {
		// Create output
		outpoint := tx.NewOutPoint(input.ReferTxID, input.ReferTxOutputIndex)
		// Try to move UTXO to STXO, if a UTXO in database was spent, it will be moved to STXO
		bc.STXOs().FromUTXO(outpoint, height, txId)
	}

	// Save transaction
	err := bc.TXNs().Put(ToStordTxn(&txn, height))
	if err != nil {
		return err
	}

	return nil
}

func (bc *Blockchain) rollbackTo(forkPoint Uint256) error {
	for {
		// Rollback header
		removed, err := bc.Headers.Rollback()
		if err != nil {
			return err
		}
		// Rollbakc TXNs and UTXOs STXOs with it
		storedTxns, err := bc.TXNs().GetAllFrom(removed.Height)
		for _, storedTxn := range storedTxns {
			var txn tx.Transaction
			err = txn.Deserialize(bytes.NewReader(storedTxn.RawData))
			if err != nil {
				return err
			}

			// Rollback UTXOs
			for index := range txn.Outputs {
				bc.UTXOs().Delete(tx.NewOutPoint(*txn.Hash(), uint16(index)))
			}

			// Rollback STXOs
			for _, input := range txn.Inputs {
				bc.STXOs().Delete(tx.NewOutPoint(input.ReferTxID, input.ReferTxOutputIndex))
			}

			bc.TXNs().Delete(&storedTxn.TxId)
		}

		if removed.Previous.IsEqual(&forkPoint) {
			return nil
		}
	}
}

func (bc *Blockchain) CheckProofOfWork(header *Header) error {
	// The target difficulty must be larger than zero.
	target := CompactToBig(header.Bits)
	if target.Sign() <= 0 {
		return errors.New("[Blockchain], block target difficulty is too low.")
	}

	// The target difficulty must be less than the maximum allowed.
	if target.Cmp(PowLimit) > 0 {
		return errors.New("[Blockchain], block target difficulty is higher than max of limit.")
	}

	// The block hash must be less than the claimed target.
	var hash Uint256

	hash = header.AuxPow.ParBlockHeader.Hash()

	hashNum := HashToBig(&hash)
	if hashNum.Cmp(target) > 0 {
		return errors.New("[Blockchain], block target difficulty is higher than expected difficulty.")
	}

	return nil
}

func HashToBig(hash *Uint256) *big.Int {
	// A Hash is in little-endian, but the big package wants the bytes in
	// big-endian, so reverse them.
	buf := *hash
	blen := len(buf)
	for i := 0; i < blen/2; i++ {
		buf[i], buf[blen-1-i] = buf[blen-1-i], buf[i]
	}

	return new(big.Int).SetBytes(buf[:])
}

func CompactToBig(compact uint32) *big.Int {
	// Extract the mantissa, sign bit, and exponent.
	mantissa := compact & 0x007fffff
	isNegative := compact&0x00800000 != 0
	exponent := uint(compact >> 24)

	// Since the base for the exponent is 256, the exponent can be treated
	// as the number of bytes to represent the full 256-bit number.  So,
	// treat the exponent as the number of bytes and shift the mantissa
	// right or left accordingly.  This is equivalent to:
	// N = mantissa * 256^(exponent-3)
	var bn *big.Int
	if exponent <= 3 {
		mantissa >>= 8 * (3 - exponent)
		bn = big.NewInt(int64(mantissa))
	} else {
		bn = big.NewInt(int64(mantissa))
		bn.Lsh(bn, 8*(exponent-3))
	}

	// Make it negative if the sign bit is set.
	if isNegative {
		bn = bn.Neg(bn)
	}

	return bn
}
