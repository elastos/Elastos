package _interface

import (
	"SPVWallet/p2p/msg"
	"SPVWallet/core/transaction"
)

type SPVService interface {
	RegisterAccount([]byte)
	OnTransactionConfirmed(func(msg.MerkleBlock, []transaction.Transaction))
	VerifyTransaction(msg.MerkleBlock, []transaction.Transaction)
	Start(clientId uint64)
}
