package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Headers interface {
	// Extend from DB interface
	DB

	// Save a header to database
	PutHeader(header *util.Header, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *util.Header) (*util.Header, error)

	// Get full header with it's hash
	GetHeader(hash *common.Uint256) (*util.Header, error)

	// Get the header on chain tip
	GetBestHeader() (*util.Header, error)

	// DelHeader delete a header save in database by it's hash.
	DelHeader(hash *common.Uint256) error
}
