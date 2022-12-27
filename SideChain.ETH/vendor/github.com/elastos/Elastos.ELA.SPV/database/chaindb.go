package database

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

type chainDB struct {
	h Headers
	t TxsDB
}

// Headers returns the headers database that stored
// all blockchain headers.
func (d *chainDB) Headers() Headers {
	return d.h
}

// CommitBlock save a block into database, returns how many
// false positive transactions are and error.
func (d *chainDB) CommitBlock(block *util.Block, newTip bool) (fps uint32, err error) {
	err = d.h.Put(&block.Header, newTip)
	if err != nil {
		return 0, err
	}

	if newTip {
		return d.t.PutTxs(block.Transactions, block.Height)
	}

	hash := block.Hash()
	return 0, d.t.PutForkTxs(block.Transactions, &hash)
}

// ProcessReorganize switch chain data to the new best chain.
func (d *chainDB) ProcessReorganize(commonAncestor, prevTip, newTip *util.Header) error {
	// 1. Move previous main chain data to fork.
	root := commonAncestor.Hash()
	header := prevTip
	hash := header.Hash()
	for !hash.IsEqual(root) {
		// Move transactions to fork.
		txs, err := d.t.GetTxs(header.Height)
		if err != nil {
			return err
		}
		err = d.t.PutForkTxs(txs, &hash)
		if err != nil {
			return err
		}

		// Delete transactions from main chain.
		err = d.t.DelTxs(header.Height)
		if err != nil {
			return err
		}

		// Move to previous header.
		header, err = d.h.GetPrevious(header)
		if err != nil {
			return err
		}
		hash = header.Hash()
	}

	// 2. Move new best chain data from fork to main chain.
	// subChain stores all fork chain headers by their previous hash, so we can
	// index next header by previous header hash.
	subChain := make(map[common.Uint256]*util.Header)
	header = newTip
	hash = header.Hash()
	for !hash.IsEqual(root) {
		// Index header by it's previous header hash.
		subChain[header.Previous()] = header

		// Move to previous header.
		var err error
		header, err = d.h.GetPrevious(header)
		if err != nil {
			return err
		}
		hash = header.Hash()
	}

	// Connect sub chain to main chain from fork root.  It is important to put
	// transactions by order, so we can process UTXOs STXOs correctly.
	tip := newTip.Hash()
	header = subChain[root]
	newRoot := header.Hash()
	for !newRoot.IsEqual(tip) {
		// Put transactions to main chain.
		txs, err := d.t.GetForkTxs(&newRoot)
		if err != nil {
			return err
		}
		_, err = d.t.PutTxs(txs, header.Height)
		if err != nil {
			return err
		}

		// Move to next header.
		header = subChain[newRoot]
		newRoot = header.Hash()
	}

	// Set new chain tip.
	return d.h.Put(newTip, true)
}

// Clear delete all data in database.
func (d *chainDB) Clear() error {
	if err := d.h.Clear(); err != nil {
		return err
	}
	if err := d.t.Clear(); err != nil {
		return err
	}
	return nil
}

// Close database.
func (d *chainDB) Close() error {
	if err := d.h.Close(); err != nil {
		return err
	}
	if err := d.t.Close(); err != nil {
		return err
	}
	return nil
}
