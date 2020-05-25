// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package indexers

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"
)

const (
	// tx3IndexName is the human-readable name for the index.
	tx3IndexName = "tx3 index"
)

var (
	// tx3IndexKey is the key of the tx3 index and the db bucket used
	// to house it.
	tx3IndexKey = []byte("tx3hash")

	// tx3IndexValue is placeholder for tx3 index
	tx3IndexValue = []byte{1}
)

func dbFetchTx3IndexEntry(dbTx database.Tx, txHash *common.Uint256) bool {
	hashIndex := dbTx.Metadata().Bucket(tx3IndexKey)
	value := hashIndex.Get(txHash[:])
	if bytes.Equal(value, tx3IndexValue) {
		return true
	}
	return false
}

func dbPutTx3IndexEntry(dbTx database.Tx, txHash *common.Uint256) error {
	tx3Index := dbTx.Metadata().Bucket(tx3IndexKey)
	return tx3Index.Put(txHash[:], tx3IndexValue)
}

// dbRemoveTxIndexEntry uses an existing database transaction to remove the most
// recent tx3 entry for the given hash.
func dbRemoveTx3IndexEntry(dbTx database.Tx, txHash *common.Uint256) error {
	tx3Index := dbTx.Metadata().Bucket(tx3IndexKey)

	return tx3Index.Delete(txHash[:])
}

// Tx3Index implements tx3 hash set which come from side chain.
type Tx3Index struct {
	db database.DB
}

// Init initializes the hash-based tx3 index. This is part of the Indexer
// interface.
func (idx *Tx3Index) Init() error {
	return nil // Nothing to do.
}

// Key returns the database key to use for the index as a byte slice.
//
// This is part of the Indexer interface.
func (idx *Tx3Index) Key() []byte {
	return tx3IndexKey
}

// Name returns the human-readable name of the index.
//
// This is part of the Indexer interface.
func (idx *Tx3Index) Name() string {
	return tx3IndexName
}

// Create is invoked when the indexer manager determines the index needs
// to be created for the first time.  It creates the buckets for the tx3
// index.
//
// This is part of the Indexer interface.
func (idx *Tx3Index) Create(dbTx database.Tx) error {
	meta := dbTx.Metadata()
	_, err := meta.CreateBucket(tx3IndexKey)
	return err
}

// ConnectBlock is invoked by the index manager when a new block has been
// connected to the main chain.  This indexer maintains a tx3 hash
// mapping for every transaction in the passed block.
//
// This is part of the Indexer interface.
func (idx *Tx3Index) ConnectBlock(dbTx database.Tx, block *types.Block) error {
	for _, txn := range block.Transactions {
		if txn.TxType != types.WithdrawFromSideChain {
			continue
		}
		witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
		for _, hash := range witPayload.SideChainTransactionHashes {
			err := dbPutTx3IndexEntry(dbTx, &hash)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

// DisconnectBlock is invoked by the index manager when a block has been
// disconnected from the main chain.  This indexer removes the
// tx3 hash mapping for every transaction in the block.
//
// This is part of the Indexer interface.
func (idx *Tx3Index) DisconnectBlock(dbTx database.Tx, block *types.Block) error {
	for _, txn := range block.Transactions {
		if txn.TxType != types.WithdrawFromSideChain {
			continue
		}
		witPayload := txn.Payload.(*payload.WithdrawFromSideChain)
		for _, hash := range witPayload.SideChainTransactionHashes {
			err := dbRemoveTx3IndexEntry(dbTx, &hash)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

// NewTx3Index returns a new instance of an indexer that is used to create a
// mapping of the program hashes of all addresses be used in the blockchain to
// the their utxo.
//
// It implements the Indexer interface which plugs into the IndexManager that in
// turn is used by the blockchain package.  This allows the index to be
// seamlessly maintained along with the chain.
func NewTx3Index(db database.DB) *Tx3Index {
	return &Tx3Index{db}
}
