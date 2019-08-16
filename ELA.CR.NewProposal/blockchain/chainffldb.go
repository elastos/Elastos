// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"bytes"
	"errors"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"
)

type ffldbChainStore struct {
	fflDB database.DB

	mtx              sync.RWMutex
	blockHashesCache []Uint256
	blocksCache      map[Uint256]*Block
}

func (c *ffldbChainStore) Close() {
	c.fflDB.Close()
}

func (c *ffldbChainStore) SaveBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	log.Debug("SaveBlock()")

	now := time.Now()
	err := c.handlePersistBlockTask(b, node, confirm, medianTimePast)

	tcall := float64(time.Now().Sub(now)) / float64(time.Second)
	log.Debugf("handle block exetime: %g num transactions:%d",
		tcall, len(b.Transactions))
	return err
}

func (c *ffldbChainStore) RollbackBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	now := time.Now()
	err := c.handleRollbackBlockTask(b, node, confirm, medianTimePast)
	tcall := float64(time.Now().Sub(now)) / float64(time.Second)
	log.Debugf("handle block rollback exetime: %g", tcall)
	return err
}

func (c *ffldbChainStore) GetBlockByHash(hash Uint256) (*Block, error) {
	var blkBytes []byte
	err := c.fflDB.View(func(dbTx database.Tx) error {
		var err error
		blkBytes, err = dbTx.FetchBlock(&hash)
		return err
	})
	if err != nil {
		return nil, err
	}

	b := new(Block)
	err = b.Deserialize(bytes.NewReader(blkBytes))
	if err != nil {
		return nil, errors.New("failed to deserialize block")
	}

	return b, nil
}

func (c *ffldbChainStore) GetHeader(hash Uint256) (*Header, error) {
	var headerBytes []byte
	err := c.fflDB.View(func(tx database.Tx) error {
		var e error
		headerBytes, e = tx.FetchBlockHeader(&hash)
		if e != nil {
			return e
		}
		return nil
	})
	if err != nil {
		return nil, errors.New("[BlockChain], GetHeader failed")
	}

	var header Header
	err = header.Deserialize(bytes.NewReader(headerBytes))
	if err != nil {
		return nil, errors.New("[BlockChain], GetHeader deserialize failed")
	}

	return &header, nil
}

func (c *ffldbChainStore) handlePersistBlockTask(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	if b.Header.Height <= c.BestChain.Height {
		return errors.New("block height less than current block height")
	}

	// Insert the block into the database if it's not already there.  Even
	// though it is possible the block will ultimately fail to connect, it
	// has already passed all proof-of-work and validity tests which means
	// it would be prohibitively expensive for an attacker to fill up the
	// disk with a bunch of blocks that fail to connect.  This is necessary
	// since it allows block download to be decoupled from the much more
	// expensive connection logic.  It also has some other nice properties
	// such as making blocks that never become part of the main chain or
	// blocks that fail to connect available for further analysis.
	err := c.fflDB.Update(func(dbTx database.Tx) error {
		return dbStoreBlock(dbTx, b)
	})
	if err != nil {
		return err
	}

	//// Write any block status changes to DB before updating best state.
	//err := b.index.flushToDB()
	//if err != nil {
	//	return err
	//}

	// Generate a new best state snapshot that will be used to update the
	// database and later memory if all database updates are successful.
	numTxns := uint64(len(b.Transactions))
	blockSize := uint64(b.GetSize())
	blockWeight := uint64(GetBlockWeight(b))
	state := newBestState(node, blockSize, blockWeight, numTxns, medianTimePast)

	// Atomically insert info into the database.
	err = c.fflDB.Update(func(dbTx database.Tx) error {
		// Update best block state.
		err := dbPutBestState(dbTx, state, node.WorkSum)
		if err != nil {
			return err
		}

		// Add the block hash and height to the block index which tracks
		// the main chain.
		blockHash := b.Hash()
		err = dbPutBlockIndex(dbTx, &blockHash, node.Height)
		if err != nil {
			return err
		}

		//// Allow the index manager to call each of the currently active
		//// optional indexes with the block being connected so they can
		//// update themselves accordingly.
		//if b.indexManager != nil {
		//	err := b.indexManager.ConnectBlock(dbTx, block, stxos)
		//	if err != nil {
		//		return err
		//	}
		//}

		return nil
	})

	if err != nil {
		return err
	}

	return c.persistConfirm(confirm)
}

func (c *ffldbChainStore) handleRollbackBlockTask(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	// Make sure the node being disconnected is the end of the best chain.
	if !node.Hash.IsEqual(*c.BestChain.Hash) {
		return errors.New("disconnectBlock must be called with the " +
			"block at the end of the main chain")
	}

	// Load the previous block since some details for it are needed below.
	prevNode := node.Parent
	var prevBlock *Block
	err := c.fflDB.View(func(dbTx database.Tx) error {
		var err error
		prevBlock, err = dbFetchBlockByNode(dbTx, prevNode)
		return err
	})
	if err != nil {
		return err
	}

	numTxns := uint64(len(prevBlock.Transactions))
	blockSize := uint64(prevBlock.GetSize())
	blockWeight := uint64(GetBlockWeight(prevBlock))
	state := newBestState(prevNode, blockSize, blockWeight, numTxns,
		medianTimePast)

	blockHash := b.Hash()

	err = c.fflDB.Update(func(dbTx database.Tx) error {
		// Update best block state.
		err := dbPutBestState(dbTx, state, node.WorkSum)
		if err != nil {
			return err
		}

		// Remove the block hash and height from the block index which
		// tracks the main chain.
		err = dbRemoveBlockIndex(dbTx, &blockHash, node.Height)
		if err != nil {
			return err
		}

		return nil
	})
	if err != nil {
		return err
	}

	return c.rollbackConfirm(confirm)
}

func (c *ffldbChainStore) persistConfirm(confirm *payload.Confirm) error {
	if confirm == nil {
		return nil
	}
	// todo complete me
	return nil
}

func (c *ffldbChainStore) rollbackConfirm(confirm *payload.Confirm) error {
	if confirm == nil {
		return nil
	}
	// todo complete me
	return nil
}

func (c *BlockChain) GetConfirm(hash Uint256) (*payload.Confirm, error) {
	// todo complete me
	return nil, nil
}

func (c *ffldbChainStore) IsBlockInStore(hash *Uint256) bool {
	var hasBlock bool
	err := c.fflDB.View(func(dbTx database.Tx) error {
		var err error
		hasBlock, err = dbTx.HasBlock(*hash)
		return err
	})
	log.Warn("[IsBlockInStore] get failed", err.Error())

	return hasBlock
}

// blockExists determines whether a block with the given hash exists either in
// the main chain or any side chains.
//
// This function is safe for concurrent access.
func (c *ffldbChainStore) blockExists(hash *Uint256) (bool, error) {
	// Check in the database.
	var exists bool
	err := c.fflDB.View(func(dbTx database.Tx) error {
		var err error
		exists, err = dbTx.HasBlock(*hash)
		if err != nil || !exists {
			return err
		}

		// Ignore side chain blocks in the database.  This is necessary
		// because there is not currently any record of the associated
		// block index data such as its block height, so it's not yet
		// possible to efficiently load the block and do anything useful
		// with it.
		//
		// Ultimately the entire block index should be serialized
		// instead of only the current main chain so it can be consulted
		// directly.
		_, err = dbFetchHeightByHash(dbTx, hash)
		if err != nil {
			exists = false
			return nil
		}
		return err
	})
	return exists, err
}
