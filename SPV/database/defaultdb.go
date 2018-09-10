package database

import "github.com/elastos/Elastos.ELA.SPV/util"

type defaultChainDB struct {
	h Headers
	t TxsDB
}

// Headers returns the headers database that stored
// all blockchain headers.
func (d *defaultChainDB) Headers() Headers {
	return d.h
}

// StoreBlock save a block into database, returns how many
// false positive transactions are and error.
func (d *defaultChainDB) StoreBlock(block *util.Block, newTip bool) (fps uint32, err error) {
	err = d.h.Put(&block.Header, newTip)
	if err != nil {
		return 0, err
	}

	// We are on a fork chain, do not commit transactions.
	if !newTip {
		return 0, nil
	}

	batch := d.t.Batch()
	for _, tx := range block.Transactions {
		fp, err := batch.AddTx(tx)
		if err != nil {
			return 0, batch.Rollback()
		}
		if fp {
			fps++
		}
	}

	return fps, batch.Commit()
}

// StoreTx save a transaction into database, and return
// if it is a false positive and error.
func (d *defaultChainDB) StoreTx(tx *util.Tx) (bool, error) {
	return d.t.CommitTx(tx)
}

// RollbackTo delete all transactions after the reorg point,
// it is used when blockchain reorganized.
func (d *defaultChainDB) Rollback(reorg *util.Header) error {
	// Get current chain tip
	best, err := d.h.GetBest()
	if err != nil {
		return err
	}

	batch := d.t.Batch()
	for current := best.Height; current > reorg.Height; current-- {
		if err := batch.DelTxs(current); err != nil {
			return batch.Rollback()
		}
	}

	if err := batch.Commit(); err != nil {
		return err
	}

	return d.h.Put(reorg, true)
}

// Clear delete all data in database.
func (d *defaultChainDB) Clear() error {
	if err := d.h.Clear(); err != nil {
		return err
	}
	if err := d.t.Clear(); err != nil {
		return err
	}
	return nil
}

// Close database.
func (d *defaultChainDB) Close() error {
	if err := d.h.Close(); err != nil {
		return err
	}
	if err := d.t.Close(); err != nil {
		return err
	}
	return nil
}
