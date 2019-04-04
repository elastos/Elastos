package sutil

import (
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/core/types"
)

var _ util.Transaction = (*Tx)(nil)

type Tx struct {
	*types.Transaction
}

func (tx *Tx) MatchFilter(bf util.Filter) bool {
	// Check if the filter matches the hash of the tx.
	// This is useful for finding transactions when they appear in a block.
	hash := tx.Hash()
	matched := bf.Matches(hash[:])

	for i, txOut := range tx.Outputs {
		if !bf.Matches(txOut.ProgramHash[:]) {
			continue
		}

		matched = true
		bf.Add(util.NewOutPoint(tx.Hash(), uint16(i)).Bytes())
	}

	// Nothing more to do if a match has already been made.
	if matched {
		return true
	}

	// At this point, the tx and none of the data elements in the
	// public key scripts of its outputs matched.

	// Check if the filter matches any outpoints this tx spends
	for _, txIn := range tx.Inputs {
		op := txIn.Previous
		if bf.Matches(util.NewOutPoint(op.TxID, op.Index).Bytes()) {
			return true
		}
	}

	return false
}

func NewTx(tx *types.Transaction) *Tx {
	return &Tx{tx}
}
