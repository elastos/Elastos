package blockchain

import (
	"errors"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	MaxBlockLocatorHashes = 100
)

var zeroHash = common.Uint256{}

var OrphanBlockError = errors.New("block does not extend any known blocks")

/*
BlockChain is the database of blocks, also when a new transaction or block commit,
BlockChain will verify them with stored blocks.
*/
type BlockChain struct {
	lock sync.RWMutex
	db   database.ChainStore
}

// NewBlockChain returns a new BlockChain instance.
func New(genesisHeader util.BlockHeader, db database.ChainStore) (*BlockChain, error) {
	// Init genesis header
	_, err := db.Headers().GetBest()
	if err != nil {
		storeHeader := &util.Header{BlockHeader: genesisHeader, TotalWork: new(big.Int)}
		if err := db.Headers().Put(storeHeader, true); err != nil {
			return nil, err
		}
	}

	return &BlockChain{db: db}, nil
}

func (b *BlockChain) CommitBlock(block *util.Block) (newTip, reorg bool, newHeight, fps uint32, err error) {
	b.lock.Lock()
	defer b.lock.Unlock()
	newTip = false
	reorg = false
	var header = &block.Header
	var commonAncestor *util.Header
	// Fetch our current best header from the db
	bestHeader, err := b.db.Headers().GetBest()
	if err != nil {
		return false, false, 0, 0, err
	}
	tipHash := bestHeader.Hash()
	var parentHeader *util.Header

	// If the tip is also the parent of this header, then we can save a database read by skipping
	// the lookup of the parent header. Otherwise (ophan?) we need to fetch the parent.
	if hash := block.Previous(); hash.IsEqual(tipHash) {
		parentHeader = bestHeader
	} else {
		parentHeader, err = b.db.Headers().GetPrevious(header)
		if err != nil {
			return false, false, 0, 0, OrphanBlockError
		}
	}
	valid := b.checkHeader(header, parentHeader)
	if !valid {
		return false, false, 0, 0, nil
	}
	// If this block is already the tip, return
	headerHash := header.Hash()
	if tipHash.IsEqual(headerHash) {
		return false, false, 0, 0, nil
	}
	// Add the work of this header to the total work stored at the previous header
	cumulativeWork := new(big.Int).Add(parentHeader.TotalWork, CalcWork(header.Bits()))

	// If the cumulative work is greater than the total work of our best header
	// then we have a new best header. Update the chain tip and check for a reorg.
	if cumulativeWork.Cmp(bestHeader.TotalWork) > 0 {
		newTip = true

		// If this header is not extending the previous best header then we have a reorg.
		if !tipHash.IsEqual(parentHeader.Hash()) {
			reorg = true
		}
	}

	// At this point, we have done header check, so store it into database.
	newHeight = parentHeader.Height + 1
	header.Height = newHeight
	header.TotalWork = cumulativeWork
	fps, err = b.db.CommitBlock(block, newTip)
	if err != nil {
		return newTip, reorg, 0, 0, err
	}

	// If not meet a reorg, just return.
	if !reorg {
		return newTip, reorg, newHeight, fps, nil
	}

	// Find common ancestor of the fork chain, so we can rollback chain to the
	// point where fork has happened.
	commonAncestor, err = b.getCommonAncestor(header, bestHeader)
	if err != nil {
		// This should not happen, because we didn't store orphan blocks in
		// database, all headers should be connected.
		log.Errorf("Error calculating common ancestor: %s", err.Error())
		return newTip, reorg, 0, 0, err
	}

	// Process block chain reorganize.
	log.Infof("REORG!!! At block %d, Wiped out %d blocks",
		bestHeader.Height, bestHeader.Height-commonAncestor.Height)
	err = b.db.ProcessReorganize(commonAncestor, bestHeader, header)
	if err != nil {
		return newTip, reorg, 0, 0, err
	}
	return newTip, reorg, newHeight, fps, nil
}

func (b *BlockChain) checkHeader(header *util.Header, prevHeader *util.Header) bool {
	// Get hash of n-1 header
	prevHash := prevHeader.Hash()
	height := prevHeader.Height

	// Check if headers link together.  That whole 'blockchain' thing.
	if prevHash.IsEqual(header.Previous()) == false {
		log.Errorf("Headers %d and %d don't link.\n", height, height+1)
		return false
	}

	// Check if there's a valid proof of work.  That whole "Bitcoin" thing.
	if !checkProofOfWork(*header) {
		log.Debugf("Block %d bad proof of work.\n", height+1)
		return false
	}

	return true // it must have worked if there's no errors and got to the end.
}

// Returns last header before reorg point
func (b *BlockChain) getCommonAncestor(bestHeader, prevTip *util.Header) (*util.Header, error) {
	var err error
	rollback := func(parent *util.Header, n int) (*util.Header, error) {
		for i := 0; i < n; i++ {
			parent, err = b.db.Headers().GetPrevious(parent)
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
		majority, err = b.db.Headers().GetPrevious(majority)
		if err != nil {
			return nil, err
		}
		minority, err = b.db.Headers().GetPrevious(minority)
		if err != nil {
			return nil, err
		}
	}
}

// HaveBlock returns whether or not the chain instance has the block represented
// by the passed hash.  This includes checking the various places a block can
// be like part of the main chain, on a side chain, or in the orphan pool.
//
// This function is safe for concurrent access.
func (b *BlockChain) HaveBlock(hash *common.Uint256) bool {
	header, _ := b.db.Headers().Get(hash)
	return header != nil
}

// LatestBlockLocator returns a block locator for current last block,
// which is a array of block hashes stored in blockchain
func (b *BlockChain) LatestBlockLocator() []*common.Uint256 {
	b.lock.RLock()
	defer b.lock.RUnlock()

	var ret []*common.Uint256
	parent, err := b.db.Headers().GetBest()
	if err != nil { // No headers stored return empty locator
		return ret
	}

	rollback := func(parent *util.Header, n int) (*util.Header, error) {
		for i := 0; i < n; i++ {
			parent, err = b.db.Headers().GetPrevious(parent)
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

// BestHeight return current best chain height.
func (b *BlockChain) BestHeight() uint32 {
	best, err := b.db.Headers().GetBest()
	if err != nil {
		return 0
	}
	return best.Height
}

// Close the blockchain
func (b *BlockChain) Clear() error {
	return b.db.Clear()
}

// Close the blockchain
func (b *BlockChain) Close() error {
	b.lock.Lock()
	return b.db.Close()
}
