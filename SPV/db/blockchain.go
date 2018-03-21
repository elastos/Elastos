package db

import (
	"errors"
	"math/big"
	"sync"

	. "SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/log"
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
	lock  *sync.RWMutex
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
		lock:      new(sync.RWMutex),
		state:     WAITING,
		Headers:   headersDB,
		DataStore: sqliteDb,
	}, nil
}

func (bc *Blockchain) Close() {
	bc.lock.Lock()
	bc.Headers.Close()
	bc.DataStore.Close()
}

func (bc *Blockchain) SetChainState(state ChainState) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	bc.state = state
}

func (bc *Blockchain) IsSyncing() bool {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.state == SYNCING
}

func (bc *Blockchain) Height() uint32 {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	tip, err := bc.Headers.GetTip()
	if err != nil {
		return 0
	}

	log.Trace("Chain height:", tip.Height)
	return tip.Height
}

func (bc *Blockchain) ChainTip() *Header {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	tip, err := bc.Headers.GetTip()
	if err != nil { // Empty blockchain, return empty header
		return new(Header)
	}
	return tip
}

func (bc *Blockchain) IsKnownBlock(hash Uint256) bool {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.isKnownBlock(hash)
}

func (bc *Blockchain) isKnownBlock(hash Uint256) bool {
	header, err := bc.Headers.GetHeader(&hash)
	if header == nil || err != nil {
		return false
	}
	return true
}

func (bc *Blockchain) GetBlockLocatorHashes() []*Uint256 {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

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
	bc.lock.Lock()
	defer bc.lock.Unlock()

	return bc.commitTxn(bc.Addrs().GetFilter(), 0, txn)
}

func (bc *Blockchain) CommitBlock(header Header, txns []tx.Transaction) error {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	// Get current chain tip
	tip, err := bc.Headers.GetTip()
	if err != nil {
		return err
	}

	// Check if commit block is a reorganize block, if so rollback to the fork point
	if header.Height < tip.Height {
		log.Debug("Blockchain rollback to:", header.Previous.String())
		err = bc.rollbackTo(header.Previous)
		if err != nil {
			return err
		}
	}

	// Save transactions first
	for _, txn := range txns {
		err := bc.commitTxn(bc.Addrs().GetFilter(), header.Height, txn)
		if err != nil {
			return err
		}
	}

	// Save header to db
	err = bc.Headers.Add(&header)
	if err != nil {
		return err
	}

	// Save current chain height
	bc.DataStore.Info().SaveChainHeight(header.Height)

	return nil
}

func (bc *Blockchain) commitTxn(filter *AddrFilter, height uint32, txn tx.Transaction) error {
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
		err = bc.DataStore.Rollback(removed.Height)
		if err != nil {
			log.Error("Rollback database failed, height: ", removed.Height, ", error: ", err)
			return err
		}

		if removed.Previous.IsEqual(&forkPoint) {
			// Save current chain height
			bc.DataStore.Info().SaveChainHeight(removed.Height - 1)
			log.Debug("Blockchain rollback finished, current height:", removed.Height-1)
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
