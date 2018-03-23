package _interface

import (
	"SPVWallet/core"
	tx "SPVWallet/core/transaction"
	"SPVWallet/db"
)

type SPVService interface {
	RegisterAccount(address string) error
	OnTransactionConfirmed(func(db.Proof, tx.Transaction))
	SubmitTransactionReceipt(txHash core.Uint256) error
	VerifyTransaction(db.Proof, tx.Transaction) error
	SendTransaction(tx.Transaction) error
	Start() error
}

func NewSPVService(clientId uint64) (SPVService, error) {
	return &SPVServiceImpl{
		clientId: clientId,
	}, nil
}
