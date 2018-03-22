package _interface

import (
	"SPVWallet/p2p/msg"
	tx "SPVWallet/core/transaction"
)

type SPVService interface {
	RegisterAccount(address string) error
	OnTransactionConfirmed(func(msg.MerkleBlock, []tx.Transaction))
	VerifyTransaction(msg.MerkleBlock, []tx.Transaction) error
	SendTransaction(tx.Transaction) error
	Start() error
}

func NewSPVService(clientId uint64) (SPVService, error) {
	return &SPVServiceImpl{
		clientId: clientId,
	}, nil
}
