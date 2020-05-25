// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2013-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"container/list"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"
)

const (
	// blockHdrOffset defines the offsets into a v1 block index row for the
	// block header.
	//
	// The serialized block index row format is:
	//   <blocklocation><blockheader>
	blockHdrOffset = 12
)

// errInterruptRequested indicates that an operation was cancelled due
// to a user-requested interrupt.
var errInterruptRequested = errors.New("interrupt requested")

var zeroHash = common.Uint256{}

// interruptRequested returns true when the provided channel has been closed.
// This simplifies early shutdown slightly since the caller can just use an if
// statement instead of a select.
func interruptRequested(interrupted <-chan struct{}) bool {
	select {
	case <-interrupted:
		return true
	default:
	}

	return false
}

// blockChainContext represents a particular block's placement in the block
// chain. This is used by the block index migration to track block metadata that
// will be written to disk.
type blockChainContext struct {
	parent    *common.Uint256
	children  []*common.Uint256
	height    int32
	mainChain bool
}

// migrateBlockIndex migrates all block entries from the v1 block index bucket
// to the v2 bucket. The v1 bucket stores all block entries keyed by block hash,
// whereas the v2 bucket stores the exact same values, but keyed instead by
// block height + hash.
func migrateBlockIndex(db database.DB) error {
	// Hardcoded bucket names so updates to the global values do not affect
	// old upgrades.
	v1BucketName := []byte("ffldb-blockidx")
	v2BucketName := []byte("blockheaderidx")

	err := db.Update(func(dbTx database.Tx) error {
		v1BlockIdxBucket := dbTx.Metadata().Bucket(v1BucketName)
		if v1BlockIdxBucket == nil {
			return fmt.Errorf("Bucket %s does not exist", v1BucketName)
		}

		log.Info("Re-indexing block information in the database. This might take a while...")

		v2BlockIdxBucket, err :=
			dbTx.Metadata().CreateBucketIfNotExists(v2BucketName)
		if err != nil {
			return err
		}

		// Get tip of the main chain.
		serializedData := dbTx.Metadata().Get(chainStateKeyName)
		state, err := deserializeBestChainState(serializedData)
		if err != nil {
			return err
		}
		tip := &state.hash

		// Scan the old block index bucket and construct a mapping of each block
		// to parent block and all child blocks.
		blocksMap, err := readBlockTree(v1BlockIdxBucket)
		if err != nil {
			return err
		}

		// Use the block graph to calculate the height of each block.
		err = determineBlockHeights(blocksMap)
		if err != nil {
			return err
		}

		// Find blocks on the main chain with the block graph and current tip.
		determineMainChainBlocks(blocksMap, tip)

		// Now that we have heights for all blocks, scan the old block index
		// bucket and insert all rows into the new one.
		return v1BlockIdxBucket.ForEach(func(hashBytes, blockRow []byte) error {
			endOffset := blockHdrOffset + blockHdrNoAuxSize
			headerBytes := blockRow[blockHdrOffset:endOffset:endOffset]

			var hash common.Uint256
			copy(hash[:], hashBytes[0:HashSize])
			chainContext := blocksMap[hash]

			if chainContext.height == -1 {
				return fmt.Errorf("Unable to calculate chain height for "+
					"stored block %s ", hash)
			}

			// Mark blocks as valid if they are part of the main chain.
			status := statusDataStored
			if chainContext.mainChain {
				status |= statusValid
			}

			// Write header to v2 bucket
			value := make([]byte, blockHdrNoAuxSize+1)
			copy(value[0:blockHdrNoAuxSize], headerBytes)
			value[blockHdrNoAuxSize] = byte(status)

			key := blockIndexKey(&hash, uint32(chainContext.height))
			err := v2BlockIdxBucket.Put(key, value)
			if err != nil {
				return err
			}

			// Delete header from v1 bucket
			truncatedRow := blockRow[0:blockHdrOffset:blockHdrOffset]
			return v1BlockIdxBucket.Put(hashBytes, truncatedRow)
		})
	})
	if err != nil {
		return err
	}

	log.Infof("Block database migration complete")
	return nil
}

// readBlockTree reads the old block index bucket and constructs a mapping of
// each block to its parent block and all child blocks. This mapping represents
// the full tree of blocks. This function does not populate the height or
// mainChain fields of the returned blockChainContext values.
func readBlockTree(v1BlockIdxBucket database.Bucket) (map[common.Uint256]*blockChainContext, error) {
	blocksMap := make(map[common.Uint256]*blockChainContext)
	err := v1BlockIdxBucket.ForEach(func(_, blockRow []byte) error {
		var header types.Header
		endOffset := blockHdrOffset + blockHdrNoAuxSize
		headerBytes := blockRow[blockHdrOffset:endOffset:endOffset]
		err := header.Deserialize(bytes.NewReader(headerBytes))
		if err != nil {
			return err
		}

		blockHash := header.Hash()
		prevHash := header.Previous

		if blocksMap[blockHash] == nil {
			blocksMap[blockHash] = &blockChainContext{height: -1}
		}
		if blocksMap[prevHash] == nil {
			blocksMap[prevHash] = &blockChainContext{height: -1}
		}

		blocksMap[blockHash].parent = &prevHash
		blocksMap[prevHash].children =
			append(blocksMap[prevHash].children, &blockHash)
		return nil
	})
	return blocksMap, err
}

// determineBlockHeights takes a map of block hashes to a slice of child hashes
// and uses it to compute the height for each block. The function assigns a
// height of 0 to the genesis hash and explores the tree of blocks
// breadth-first, assigning a height to every block with a path back to the
// genesis block. This function modifies the height field on the blocksMap
// entries.
func determineBlockHeights(blocksMap map[common.Uint256]*blockChainContext) error {
	queue := list.New()

	// The genesis block is included in blocksMap as a child of the zero hash
	// because that is the value of the PrevBlock field in the genesis header.
	preGenesisContext, exists := blocksMap[common.Uint256{}]
	if !exists || len(preGenesisContext.children) == 0 {
		return fmt.Errorf("Unable to find genesis block")
	}

	for _, genesisHash := range preGenesisContext.children {
		blocksMap[*genesisHash].height = 0
		queue.PushBack(genesisHash)
	}

	for e := queue.Front(); e != nil; e = queue.Front() {
		queue.Remove(e)
		hash := e.Value.(*common.Uint256)
		height := blocksMap[*hash].height

		// For each block with this one as a parent, assign it a height and
		// push to queue for future processing.
		for _, childHash := range blocksMap[*hash].children {
			blocksMap[*childHash].height = height + 1
			queue.PushBack(childHash)
		}
	}

	return nil
}

// determineMainChainBlocks traverses the block graph down from the tip to
// determine which block hashes that are part of the main chain. This function
// modifies the mainChain field on the blocksMap entries.
func determineMainChainBlocks(blocksMap map[common.Uint256]*blockChainContext, tip *common.Uint256) {
	for nextHash := tip; *nextHash != zeroHash; nextHash = blocksMap[*nextHash].parent {
		blocksMap[*nextHash].mainChain = true
	}
}
