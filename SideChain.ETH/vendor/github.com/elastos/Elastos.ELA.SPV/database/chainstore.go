package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"
)

type ChainStore interface {
	// Extend from DB interface
	DB

	// Headers returns the headers database that stored
	// all blockchain headers.
	Headers() Headers

	// CommitBlock save a block into database, returns how many
	// false positive transactions are and error.
	CommitBlock(block *util.Block, newTip bool) (fps uint32, err error)

	// ProcessReorganize switch chain data to the new best chain.
	ProcessReorganize(commonAncestor, prevTip, newTip *util.Header) error
}

func NewChainDB(h Headers, t TxsDB) ChainStore {
	return &chainDB{h: h, t: t}
}
