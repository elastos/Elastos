package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

type Headers interface {
	// Extend from DB interface
	DB

	// Save a header to database
	Put(header *util.Header, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *util.Header) (*util.Header, error)

	// Get full header with it's hash
	Get(hash *common.Uint256) (*util.Header, error)

	// Get the header on chain tip
	GetBest() (*util.Header, error)
}
