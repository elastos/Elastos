package sdk

import (
	"errors"
	"math/big"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/db"
	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA/bloom"
	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type ChainState int

const (
	SYNCING = ChainState(0)
	WAITING = ChainState(1)
)

const (
	MaxBlockLocatorHashes = 100
)

var PowLimit = new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 255), big.NewInt(1))

/*
StateListener is an interface to listen blockchain data change.
Call AddStateListener() method to register your callbacks to the notify list.
*/
type StateListener interface {
	// This method will be callback after a transaction committed
	// Notice: this method will be called when commit block
	OnTxCommitted(tx Transaction, height uint32)

	// This method will be callback after a block committed
	OnBlockCommitted(bloom.MerkleBlock, []Transaction)

	// This method will be callback when blockchain rollback
	// height is the deleted data height, for example OnChainRollback(100) means
	// data on height 100 has been deleted, current chain height will be 99.
	OnChainRollback(height uint32)
}

/*
Blockchain is the database of blocks, also when a new transaction or block commit,
Blockchain will verify them with stored blocks.
*/
type Blockchain struct {
	lock           *sync.RWMutex
	state          ChainState
	db.DataStore
	stateListeners []StateListener
}

// Create a instance of *Blockchain
func NewBlockchain(dataStore db.DataStore) (*Blockchain, error) {
	return &Blockchain{
		lock:      new(sync.RWMutex),
		state:     WAITING,
		DataStore: dataStore,
	}, nil
}

// Register a blockchain state listener, multiple registration is supported.
func (bc *Blockchain) AddStateListener(listener StateListener) {
	bc.stateListeners = append(bc.stateListeners, listener)
}

// Close the blockchain
func (bc *Blockchain) Close() {
	bc.lock.Lock()
	bc.DataStore.Close()
}

// Set the current state of blockchain
func (bc *Blockchain) SetChainState(state ChainState) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	bc.state = state
}

// Return a bool value if blockchain is in syncing state
func (bc *Blockchain) IsSyncing() bool {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.state == SYNCING
}

// Get current blockchain height
func (bc *Blockchain) Height() uint32 {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.DataStore.GetChainHeight()
}

// Get current blockchain tip
func (bc *Blockchain) ChainTip() *db.StoreHeader {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	return bc.chainTip()
}

func (bc *Blockchain) chainTip() *db.StoreHeader {
	tip, err := bc.GetChainTip()
	if err != nil { // Empty blockchain, return empty header
		return &db.StoreHeader{TotalWork: new(big.Int)}
	}
	return tip
}

// Create a block locator which is a array of block hashes stored in blockchain
func (bc *Blockchain) GetBlockLocatorHashes() []*Uint256 {
	bc.lock.RLock()
	defer bc.lock.RUnlock()

	var ret []*Uint256
	parent, err := bc.GetChainTip()
	if err != nil { // No headers stored return empty locator
		return ret
	}

	rollback := func(parent *db.StoreHeader, n int) (*db.StoreHeader, error) {
		for i := 0; i < n; i++ {
			parent, err = bc.GetPrevious(parent)
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
		ret = append(ret, &hash)
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

// Commit tx commits a transaction and return is false positive and error
func (bc *Blockchain) CommitTx(tx Transaction) (bool, error) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	return bc.commitTx(tx, 0)
}

// Commit block commits a block and transactions with it, return is reorganize, false positives and error
func (bc *Blockchain) CommitBlock(block bloom.MerkleBlock, txs []Transaction) (bool, int, error) {
	bc.lock.Lock()
	defer bc.lock.Unlock()

	header := block.Header
	commitHeader := &db.StoreHeader{Header: header}

	// Get current chain tip
	tip := bc.chainTip()
	tipHash := tip.Hash()

	// Lookup of the parent header. Otherwise (ophan?) we need to fetch the parent.
	// If the tip is also the parent of this header, then we can save a database read by skipping
	var err error
	var newTip = false
	var parentHeader *db.StoreHeader
	if header.Previous.IsEqual(tipHash) {
		parentHeader = tip
	} else {
		parentHeader, err = bc.GetPrevious(commitHeader)
		if err != nil {
			// If committing header is genesis header, make an empty parent header
			if commitHeader.Height == 1 {
				parentHeader = &db.StoreHeader{TotalWork: new(big.Int)}
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
	var reorgPoint *db.StoreHeader
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
		err = bc.PutHeader(reorgPoint, newTip)
		if err != nil {
			return reorg, 0, err
		}
		return true, 0, nil
	}

	fPositives := 0
	if newTip {
		// Save transactions
		for _, tx := range txs {
			fPositive, err := bc.commitTx(tx, header.Height)
			if err != nil {
				return reorg, 0, err
			}
			if fPositive {
				fPositives++
			}
		}
		// Save current chain height
		bc.DataStore.PutChainHeight(header.Height)
	}

	log.Debug("Commit header: ", commitHeader.Hash().String(), ", newTip: ", newTip)
	// Save header to db
	err = bc.PutHeader(commitHeader, newTip)
	if err != nil {
		return reorg, 0, err
	}

	// Notify block committed
	bc.notifyBlockCommitted(block, txs)

	log.Debug("Blockchain block committed height: ", bc.chainTip().Height)

	return reorg, fPositives, nil
}

func (bc *Blockchain) commitTx(tx Transaction, height uint32) (bool, error) {
	fPositive, err := bc.DataStore.CommitTx(db.NewStoreTx(tx, height))
	if err != nil {
		return false, err
	}

	bc.notifyTxCommitted(tx, height)

	return fPositive, nil
}

// Rollback data store to the fork point
func (bc *Blockchain) rollbackTo(forkPoint uint32) error {
	for height := bc.DataStore.GetChainHeight(); height > forkPoint; height-- {
		// Rollback TXNs and UTXOs STXOs with it
		err := bc.DataStore.Rollback(height)
		if err != nil {
			fmt.Println("Rollback database failed, height: ", height, ", error: ", err)
			return err
		}
		bc.notifyChainRollback(height)
	}
	// Save current chain height
	bc.DataStore.PutChainHeight(forkPoint)

	return nil
}

// Returns last header before reorg point
func (bc *Blockchain) getCommonAncestor(bestHeader, prevTip *db.StoreHeader) (*db.StoreHeader, error) {
	var err error
	rollback := func(parent *db.StoreHeader, n int) (*db.StoreHeader, error) {
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

func (bc *Blockchain) notifyBlockCommitted(block bloom.MerkleBlock, txs []Transaction) {
	for _, listener := range bc.stateListeners {
		go listener.OnBlockCommitted(block, txs)
	}
}

func (bc *Blockchain) notifyTxCommitted(tx Transaction, height uint32) {
	for _, listener := range bc.stateListeners {
		go listener.OnTxCommitted(tx, height)
	}
}

func (bc *Blockchain) notifyChainRollback(height uint32) {
	for _, listener := range bc.stateListeners {
		go listener.OnChainRollback(height)
	}
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

func (bc *Blockchain) CheckProofOfWork(header Header) error {
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
	hash := header.AuxPow.ParBlockHeader.Hash()
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
