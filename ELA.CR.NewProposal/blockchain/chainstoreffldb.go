// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package blockchain

import (
	"bytes"
	"errors"
	"os"
	"path/filepath"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/blockchain/indexers"
	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"

	"github.com/btcsuite/btcd/wire"
)

const (
	// blockDbName is the block database name.
	blockDbName = "blocks"

	// oldBlockDbName is the old block database name.
	oldBlockDbName = "blocks_ffldb"

	BlocksCacheSize = 2
)

type ChainStoreFFLDB struct {
	db database.DB

	indexManager indexers.IndexManager

	mtx              sync.RWMutex
	blockHashesCache []Uint256
	blocksCache      map[Uint256]*DposBlock
}

func NewChainStoreFFLDB(dataDir string, params *config.Params) (IFFLDBChainStore, error) {
	fflDB, err := LoadBlockDB(dataDir, blockDbName)
	if err != nil {
		return nil, err
	}
	indexManager := indexers.NewManager(fflDB, params)

	s := &ChainStoreFFLDB{
		db:               fflDB,
		indexManager:     indexManager,
		blockHashesCache: make([]Uint256, 0, BlocksCacheSize),
		blocksCache:      make(map[Uint256]*DposBlock),
	}

	return s, nil
}

// dbPath returns the path to the block database given a database type.
func blockDbPath(dataPath, dbName string) string {
	// The database name is based on the database type.
	dbPath := filepath.Join(dataPath, dbName)
	return dbPath
}

// loadBlockDB loads (or creates when needed) the block database taking into
// account the selected database backend and returns a handle to it.  It also
// contains additional logic such warning the user if there are multiple
// databases which consume space on the file system and ensuring the regression
// test database is clean when in regression test mode.
func LoadBlockDB(dataPath string, dbName string) (database.DB, error) {
	// The memdb backend does not have a file path associated with it, so
	// handle it uniquely.  We also don't want to worry about the multiple
	// database type warnings when running with the memory database.

	// The database name is based on the database type.
	dbType := "ffldb"
	dbPath := blockDbPath(dataPath, dbName)

	log.Infof("Loading block database from '%s'", dbPath)
	db, err := database.Open(dbType, dbPath, wire.MainNet)
	if err != nil {
		// Return the error if it's not because the database doesn't
		// exist.
		if dbErr, ok := err.(database.Error); !ok || dbErr.ErrorCode !=
			database.ErrDbDoesNotExist {

			return nil, err
		}

		// Create the db if it does not exist.
		err = os.MkdirAll(dataPath, 0700)
		if err != nil {
			return nil, err
		}
		db, err = database.Create(dbType, dbPath, wire.MainNet)
		if err != nil {
			return nil, err
		}
	}

	log.Info("Block database loaded")
	return db, nil
}

func (c *ChainStoreFFLDB) Type() string {
	return c.db.Type()
}

func (c *ChainStoreFFLDB) Begin(writable bool) (database.Tx, error) {
	return c.db.Begin(writable)
}

func (c *ChainStoreFFLDB) View(fn func(tx database.Tx) error) error {
	return c.db.View(fn)
}

func (c *ChainStoreFFLDB) Update(fn func(tx database.Tx) error) error {
	return c.db.Update(fn)
}

func (c *ChainStoreFFLDB) Close() error {
	return c.db.Close()
}

func (c *ChainStoreFFLDB) SaveBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {

	err := c.db.Update(func(dbTx database.Tx) error {
		return dbStoreBlock(dbTx, &DposBlock{
			Block:       b,
			HaveConfirm: confirm != nil,
			Confirm:     confirm,
		})
	})
	if err != nil {
		return err
	}

	// Generate a new best state snapshot that will be used to update the
	// database and later memory if all database updates are successful.
	numTxns := uint64(len(b.Transactions))
	blockSize := uint64(b.GetSize())
	blockWeight := uint64(GetBlockWeight(b))
	state := newBestState(node, blockSize, blockWeight, numTxns, medianTimePast)

	// Atomically insert info into the database.
	err = c.db.Update(func(dbTx database.Tx) error {
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

		// Allow the index manager to call each of the currently active
		// optional indexes with the block being connected so they can
		// update themselves accordingly.
		if c.indexManager != nil {
			err := c.indexManager.ConnectBlock(dbTx, b)
			if err != nil {
				return err
			}
		}

		return nil
	})

	return err
}

func (c *ChainStoreFFLDB) RollbackBlock(b *Block, node *BlockNode,
	confirm *payload.Confirm, medianTimePast time.Time) error {
	// Load the previous block since some details for it are needed below.
	prevNode := node.Parent
	var prevBlock *Block
	err := c.db.View(func(dbTx database.Tx) error {
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

	err = c.db.Update(func(dbTx database.Tx) error {
		// Update best block state.
		err := dbPutBestState(dbTx, state, node.WorkSum)
		if err != nil {
			return err
		}

		// Remove the block hash and height from the block index which
		// tracks the main chain.
		err = DBRemoveBlockIndex(dbTx, &blockHash, node.Height)
		if err != nil {
			return err
		}

		// Allow the index manager to call each of the currently active
		// optional indexes with the block being disconnected so they
		// can update themselves accordingly.
		if c.indexManager != nil {
			err := c.indexManager.DisconnectBlock(dbTx, b)
			if err != nil {
				return err
			}
		}

		return nil
	})

	return err
}

func (c *ChainStoreFFLDB) GetOldBlock(hash Uint256) (*Block, error) {
	var blkBytes []byte
	err := c.db.View(func(dbTx database.Tx) error {
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

func (c *ChainStoreFFLDB) GetBlock(hash Uint256) (*DposBlock, error) {
	c.mtx.RLock()
	if block, exist := c.blocksCache[hash]; exist {
		c.mtx.RUnlock()
		return block, nil
	}
	c.mtx.RUnlock()

	var blkBytes []byte
	err := c.db.View(func(dbTx database.Tx) error {
		var err error
		blkBytes, err = dbTx.FetchBlock(&hash)
		return err
	})
	if err != nil {
		return nil, err
	}

	b := new(DposBlock)
	err = b.Deserialize(bytes.NewReader(blkBytes))
	if err != nil {
		return nil, errors.New("failed to deserialize block")
	}

	c.mtx.Lock()
	if c.blocksCache != nil {
		if len(c.blockHashesCache) >= BlocksCacheSize {
			delete(c.blocksCache, c.blockHashesCache[0])
			c.blockHashesCache = c.blockHashesCache[1:BlocksCacheSize]
		}
		c.blockHashesCache = append(c.blockHashesCache, hash)
		c.blocksCache[hash] = b
	}
	c.mtx.Unlock()

	return b, nil
}

func (c *ChainStoreFFLDB) GetHeader(hash Uint256) (*Header, error) {
	var headerBytes []byte
	err := c.db.View(func(tx database.Tx) error {
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
	err = header.DeserializeNoAux(bytes.NewReader(headerBytes))
	if err != nil {
		return nil, errors.New("[BlockChain], GetHeader deserialize failed")
	}

	return &header, nil
}

func (c *ChainStoreFFLDB) IsBlockInStore(hash *Uint256) bool {
	var hasBlock bool
	err := c.db.View(func(dbTx database.Tx) error {
		var err error
		hasBlock, err = dbTx.HasBlock(*hash)
		return err
	})
	if err != nil {
		log.Warn("[IsBlockInStore] get failed", err.Error())
	}

	return hasBlock
}

// blockExists determines whether a block with the given hash exists either in
// the main chain or any side chains.
//
// This function is safe for concurrent access.
func (c *ChainStoreFFLDB) BlockExists(hash *Uint256) (bool, uint32, error) {
	// Check in the database.
	var exists bool
	var height uint32
	err := c.db.View(func(dbTx database.Tx) error {
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
		height, err = dbFetchHeightByHash(dbTx, hash)
		if err != nil {
			exists = false
			return nil
		}
		return err
	})
	return exists, height, err
}

func (c *ChainStoreFFLDB) GetTransaction(txID Uint256) (*Transaction, uint32, error) {
	return c.indexManager.FetchTx(txID)
}

func (c *ChainStoreFFLDB) InitIndex(chain indexers.IChain, interrupt <-chan struct{}) error {
	return c.indexManager.Init(chain, interrupt)
}

func (c *ChainStoreFFLDB) GetUnspent(txID Uint256) ([]uint16, error) {
	return c.indexManager.FetchUnspent(txID)
}

func (c *ChainStoreFFLDB) GetUTXO(programHash *Uint168) ([]*UTXO, error) {
	return c.indexManager.FetchUTXO(programHash)
}

func (c *ChainStoreFFLDB) IsTx3Exist(txHash *Uint256) bool {
	return c.indexManager.IsTx3Exist(txHash)
}
