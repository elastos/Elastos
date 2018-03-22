package spv

import (
	"SPVWallet/p2p/msg"
	tx "SPVWallet/core/transaction"
)

func (spv *SPV) SetOnTxCommitListener(listener func(txn tx.Transaction)) {
	if listener == nil {
		return
	}
	spv.OnTxCommit = listener
}

func (spv *SPV) SetOnBlockCommitListener(listener func(msg.MerkleBlock, []tx.Transaction)) {
	if listener == nil {
		return
	}
	spv.OnBlockCommit = listener
}

func (spv *SPV) SetOnRollbackListener(listener func(uint32)) {
	if listener == nil {
		return
	}
	spv.chain.OnRollback = listener
}

func OnTxCommit(tx.Transaction) {}

func OnBlockCommit(msg.MerkleBlock, []tx.Transaction) {}

func OnRollback(uint32) {}
