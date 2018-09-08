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

	// StoreBlock save a block into database, returns how many
	// false positive transactions are and error.
	StoreBlock(block *util.Block, newTip bool) (fps uint32, err error)

	// StoreTx save a transaction into database, and return
	// if it is a false positive and error.
	StoreTx(tx *util.Tx) (bool, error)

	// Rollback delete all transactions after the reorg point,
	// it is used when blockchain reorganized.
	Rollback(reorg *util.Header)  error
}

func NewHeadersOnlyChainDB(db Headers) ChainStore {
	return &headersOnlyChainDB{db: db}
}

func NewDefaultChainDB(h Headers, t TxsDB) ChainStore {
	return &defaultChainDB{h: h, t: t}
}
