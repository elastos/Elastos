package db

import (
	"github.com/elastos/Elastos.ELA.SPV/common"
)

type DataStore interface {
	// Save a header to database
	PutHeader(header *StoreHeader, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *StoreHeader) (*StoreHeader, error)

	// Get full header with it's hash
	GetHeader(hash common.Uint256) (*StoreHeader, error)

	// Get the header on chain tip
	GetChainTip() (*StoreHeader, error)

	// Save chain height to database
	PutChainHeight(height uint32)

	// Get chain height from database
	GetChainHeight() uint32

	// Commit a transaction return if this is a false positive and error
	CommitTx(tx *StoreTx) (bool, error)

	// Rollback chain data on the given height
	Rollback(height uint32) error

	// Reset database, clear all data
	Reset() error

	// Close the database
	Close()
}
