package store

import "github.com/elastos/Elastos.ELA.Utility/common"

type HeaderStore interface {
	// Save a header to database
	PutHeader(header *StoreHeader, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *StoreHeader) (*StoreHeader, error)

	// Get full header with it's hash
	GetHeader(hash *common.Uint256) (*StoreHeader, error)

	// Get the header on chain tip
	GetBestHeader() (*StoreHeader, error)

	// Reset header store
	Reset() error

	// Close header store
	Close()
}
