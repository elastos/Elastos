package spvwallet

import (
	. "github.com/elastos/Elastos.ELA.SPV/core"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
)

func (wallet *SPVWallet) SetOnTxCommitListener(listener func(tx.Transaction)) {
	if listener == nil {
		return
	}
	wallet.chain.OnTxCommit = listener
}

func (wallet *SPVWallet) SetOnBlockCommitListener(listener func(Header, db.Proof, []tx.Transaction)) {
	if listener == nil {
		return
	}
	wallet.chain.OnBlockCommit = listener
}

func (wallet *SPVWallet) SetOnRollbackListener(listener func(uint32)) {
	if listener == nil {
		return
	}
	wallet.chain.OnRollback = listener
}

func OnTxCommit(tx.Transaction) {}

func OnBlockCommit(Header, db.Proof, []tx.Transaction) {}

func OnRollback(uint32) {}
