package spv

import (
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
)

func (spv *SPV) SetOnTxCommitListener(listener func(txn tx.Transaction)) {
	if listener == nil {
		return
	}
	spv.chain.OnTxCommit = listener
}

func (spv *SPV) SetOnBlockCommitListener(listener func(db.Header, db.Proof, []tx.Transaction)) {
	if listener == nil {
		return
	}
	spv.chain.OnBlockCommit = listener
}

func (spv *SPV) SetOnRollbackListener(listener func(uint32)) {
	if listener == nil {
		return
	}
	spv.chain.OnRollback = listener
}

func OnTxCommit(tx.Transaction) {}

func OnBlockCommit(db.Header, db.Proof, []tx.Transaction) {}

func OnRollback(uint32) {}
