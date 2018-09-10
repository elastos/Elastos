package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"
)

type headersOnlyChainDB struct {
	db Headers
}

// Headers returns the headers database that stored
// all blockchain headers.
func (h *headersOnlyChainDB) Headers() Headers {
	return h.db
}

// StoreBlock save a block into database, returns how many
// false positive transactions are and error.
func (h *headersOnlyChainDB) StoreBlock(block *util.Block, newTip bool) (fps uint32, err error) {
	return fps, h.db.Put(&block.Header, newTip)
}

// StoreTx save a transaction into database, and return
// if it is a false positive and error.
func (h *headersOnlyChainDB) StoreTx(tx *util.Tx) (bool, error) {
	return false, nil
}

// RollbackTo delete all transactions after the reorg point,
// it is used when blockchain reorganized.
func (h *headersOnlyChainDB) Rollback(reorg *util.Header) error {
	// Just do nothing.  Headers never removed from database,
	// only transactions need to be rollback.
	return nil
}

// Clear delete all data in database.
func (h *headersOnlyChainDB) Clear() error {
	return h.db.Clear()
}

// Close database.
func (h *headersOnlyChainDB) Close() error {
	return h.db.Close()
}
