package spvwallet

import (
	"bytes"
	"errors"
	"math/big"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/core"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
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
	lock             *sync.RWMutex
	state            ChainState
	db.Headers
	db.Proofs
	db.DataStore
	OnBlockCommitted func(core.Header, db.Proof, []tx.Transaction)
}

func NewBlockchain() (*Blockchain, error) {
	headersDB, err := db.NewHeadersDB()
	if err != nil {
		return nil, err
	}

	proofsDB, err := db.NewProofsDB()
	if err != nil {
		return nil, err
	}

	sqliteDb, err := db.NewSQLiteDB()
	if err != nil {
		return nil, err
	}

	return &Blockchain{
		lock:      new(sync.RWMutex),
		state:     WAITING,
		Headers:   headersDB,
		Proofs:    proofsDB,
		DataStore: sqliteDb,
	}, nil
}

func (bc *Blockchain) Close() {
	bc.lock.Lock()
	bc.Headers.Close()
	bc.Proofs.Close()
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

	return bc.DataStore.Info().ChainHeight()
}

func (bc *Blockchain) ChainTip() *db.Header {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.chainTip()
}

func (bc *Blockchain) chainTip() *db.Header {
	tip, err := bc.Headers.GetTip()
	if err != nil { // Empty blockchain, return empty header
		return &db.Header{TotalWork: new(big.Int)}
	}
	return tip
}

func (bc *Blockchain) IsKnownBlock(hash Uint256) bool {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	header, err := bc.Headers.GetHeader(hash)
	if header == nil || err != nil {
		return false
	}
	return true
}

func (bc *Blockchain) GetBloomFilter() *bloom.Filter {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	addrs := bc.Addrs().GetAddrFilter().GetAddrs()
	utxos, _ := bc.UTXOs().GetAll()
	stxos, _ := bc.STXOs().GetAll()

	elements := uint32(len(addrs) + len(utxos) + len(stxos))
	filter := sdk.NewBloomFilter(elements)

	for _, addr := range addrs {
		filter.Add(addr.ToArray())
	}

	for _, utxo := range utxos {
		filter.AddOutPoint(&utxo.Op)
	}

	for _, stxo := range stxos {
		filter.AddOutPoint(&stxo.Op)
	}

	return filter
}

func (bc *Blockchain) GetBlockLocatorHashes() []*Uint256 {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	var ret []*Uint256
	parent, err := bc.Headers.GetTip()
	if err != nil { // No headers stored return empty locator
		return ret
	}

	rollback := func(parent *db.Header, n int) (*db.Header, error) {
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

func (bc *Blockchain) CommitUnconfirmedTxn(txn tx.Transaction) (bool, error) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	return bc.commitTxn(0, txn)
}

// Commit block commits a block and transactions with it, return is reorganize, false positives and error
func (bc *Blockchain) CommitBlock(header core.Header, proof db.Proof, txns []tx.Transaction) (bool, int, error) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	commitHeader := &db.Header{Header: header}

	// Get current chain tip
	tip := bc.chainTip()
	tipHash := tip.Hash()

	// Lookup of the parent header. Otherwise (ophan?) we need to fetch the parent.
	// If the tip is also the parent of this header, then we can save a database read by skipping
	var err error
	var newTip = false
	var parentHeader *db.Header
	if header.Previous.IsEqual(tipHash) {
		parentHeader = tip
	} else {
		parentHeader, err = bc.GetPrevious(commitHeader)
		if err != nil {
			// If committing header is genesis header, make an empty parent header
			if commitHeader.Height == 1 {
				parentHeader = &db.Header{TotalWork: new(big.Int)}
			} else {
				return false, 0, fmt.Errorf("Header %s does not extend any known headers", header.Hash().String())
			}
		}
	}

	log.Debug("Find parent header height: ", parentHeader.Height)

	// If this block is already the tip, return
	if tipHash.IsEqual(header.Hash()) {
		return false, 0, nil
	}
	// Add the work of this header to the total work stored at the previous header
	cumulativeWork := new(big.Int).Add(parentHeader.TotalWork, CalcWork(header.Bits))
	commitHeader.TotalWork = cumulativeWork

	// If the cumulative work is greater than the total work of our best header
	// then we have a new best header. Update the chain tip and check for a reorg.
	var reorg = false
	var reorgPoint *db.Header
	if cumulativeWork.Cmp(tip.TotalWork) == 1 {
		newTip = true
		// If this header is not extending the previous best header then we have a reorg.
		if !tipHash.IsEqual(parentHeader.Hash()) {
			commitHeader.Height = parentHeader.Height + 1
			reorgPoint, err = bc.getCommonAncestor(commitHeader, tip)
			if err != nil {
				log.Errorf("error calculating common ancestor: %s", err.Error())
				return false, 0, err
			}
			fmt.Printf("Reorganize At block %d, Wiped out %d blocks\n",
				int(tip.Height), int(tip.Height-reorgPoint.Height))
		}
	}

	// If common ancestor exists, means we have an fork chan
	// so we need to rollback to the last good point.
	if reorgPoint != nil {
		log.Warn("Meet reorganize rollback to: ", reorgPoint.Height)
		err := bc.rollbackTo(reorgPoint.Height)
		if err != nil {
			fmt.Println(err)
		}
		// Save reorganize point as the new tip
		err = bc.Headers.Put(reorgPoint, newTip)
		if err != nil {
			return reorg, 0, err
		}
		return true, 0, nil
	}

	fPositives := 0
	if newTip {
		// Save transactions
		for _, txn := range txns {
			fPositive, err := bc.commitTxn(header.Height, txn)
			if err != nil {
				return reorg, 0, err
			}
			if fPositive {
				fPositives++
			}
		}
		// Save merkle proof
		err = bc.Proofs.Put(&proof)
		if err != nil {
			return reorg, 0, err
		}
		// Save current chain height
		bc.DataStore.Info().SaveChainHeight(header.Height)
	}

	log.Debug("Commit header: ", commitHeader.Hash().String(), ", newTip: ", newTip)
	// Save header to db
	err = bc.Headers.Put(commitHeader, newTip)
	if err != nil {
		return reorg, 0, err
	}

	// Notify block committed
	go bc.OnBlockCommitted(header, proof, txns)

	log.Debug("Blockchain block committed height: ", bc.chainTip().Height)

	return reorg, fPositives, nil
}

// Rollback data store to the fork point
func (bc *Blockchain) rollbackTo(forkPoint uint32) error {
	for height := bc.DataStore.Info().ChainHeight(); height > forkPoint; height-- {
		// Rollback TXNs and UTXOs STXOs with it
		err := bc.DataStore.Rollback(height)
		if err != nil {
			fmt.Println("Rollback database failed, height: ", height, ", error: ", err)
			return err
		}
	}
	// Save current chain height
	bc.DataStore.Info().SaveChainHeight(forkPoint)

	return nil
}

// Commit a transaction to database, return if the committed transaction is a false positive
func (bc *Blockchain) commitTxn(height uint32, txn tx.Transaction) (bool, error) {
	txId := txn.Hash()
	hits := 0
	// Save UTXOs
	for index, output := range txn.Outputs {
		// Filter address
		if bc.Addrs().GetAddrFilter().ContainAddr(output.ProgramHash) {
			var lockTime uint32
			if txn.TxType == tx.CoinBase {
				lockTime = height + 100
			}
			utxo := ToUTXO(txId, height, index, output.Value, lockTime)
			err := bc.UTXOs().Put(&output.ProgramHash, utxo)
			if err != nil {
				return false, err
			}
			hits++
		}
	}

	// Put spent UTXOs to STXOs
	for _, input := range txn.Inputs {
		// Create output
		outpoint := tx.NewOutPoint(input.ReferTxID, input.ReferTxOutputIndex)
		// Try to move UTXO to STXO, if a UTXO in database was spent, it will be moved to STXO
		err := bc.STXOs().FromUTXO(outpoint, txId, height)
		if err == nil {
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction
	err := bc.TXNs().Put(ToTxn(txn, height))
	if err != nil {
		return false, err
	}

	return false, nil
}

// Returns last header before reorg point
func (bc *Blockchain) getCommonAncestor(bestHeader, prevTip *db.Header) (*db.Header, error) {
	var err error
	rollback := func(parent *db.Header, n int) (*db.Header, error) {
		for i := 0; i < n; i++ {
			parent, err = bc.GetPrevious(parent)
			if err != nil {
				return parent, err
			}
		}
		return parent, nil
	}

	majority := bestHeader
	minority := prevTip
	if bestHeader.Height > prevTip.Height {
		majority, err = rollback(majority, int(bestHeader.Height-prevTip.Height))
		if err != nil {
			return nil, err
		}
	} else if prevTip.Height > bestHeader.Height {
		minority, err = rollback(minority, int(prevTip.Height-bestHeader.Height))
		if err != nil {
			return nil, err
		}
	}

	for {
		majorityHash := majority.Hash()
		minorityHash := minority.Hash()
		if majorityHash.IsEqual(minorityHash) {
			return majority, nil
		}
		majority, err = bc.GetPrevious(majority)
		if err != nil {
			return nil, err
		}
		minority, err = bc.GetPrevious(minority)
		if err != nil {
			return nil, err
		}
	}
}

func ToTxn(tx tx.Transaction, height uint32) *db.Txn {
	txn := new(db.Txn)
	txn.TxId = *tx.Hash()
	txn.Height = height
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	txn.RawData = buf.Bytes()
	return txn
}

func ToUTXO(txId *Uint256, height uint32, index int, value Fixed64, lockTime uint32) *db.UTXO {
	utxo := new(db.UTXO)
	utxo.Op = *tx.NewOutPoint(*txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	return utxo
}

func InputFromUTXO(utxo *db.UTXO) *tx.Input {
	input := new(tx.Input)
	input.ReferTxID = utxo.Op.TxID
	input.ReferTxOutputIndex = utxo.Op.Index
	input.Sequence = utxo.LockTime
	return input
}

func CalcWork(bits uint32) *big.Int {
	// Return a work value of zero if the passed difficulty bits represent
	// a negative number. Note this should not happen in practice with valid
	// blocks, but an invalid block could trigger it.
	difficultyNum := CompactToBig(bits)
	if difficultyNum.Sign() <= 0 {
		return big.NewInt(0)
	}

	// (1 << 256) / (difficultyNum + 1)
	denominator := new(big.Int).Add(difficultyNum, big.NewInt(1))
	return new(big.Int).Div(new(big.Int).Lsh(big.NewInt(1), 256), denominator)
}

func (bc *Blockchain) CheckProofOfWork(header *core.Header) error {
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
