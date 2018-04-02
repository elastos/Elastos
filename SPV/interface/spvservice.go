package _interface

import (
	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
)

/*
SPV service is the interface to interactive with the SPV (Simplified Payment Verification)
service implementation running background, you can register specific accounts that you are
interested in and receive transaction notifications of these accounts.
*/
type SPVService interface {
	// Register the account address that you are interested in
	RegisterAccount(address string) error

	// Register the callback method to receive transaction notifications
	// when a transaction related with the registered accounts is confirmed
	// The merkle tree proof and the transaction will be callback
	OnTransactionConfirmed(func(db.Proof, tx.Transaction))

	// After receive the transaction callback, call this method
	// to confirm that the transaction with the given ID was handled
	// so the transaction will be removed from the notify queue
	SubmitTransactionReceipt(txId Uint256) error

	// To verify if a transaction is valid
	// This method is useful when receive a transaction from other peer
	VerifyTransaction(db.Proof, tx.Transaction) error

	// Send a transaction to the P2P network
	SendTransaction(tx.Transaction) error

	// Start the SPV service
	Start() error
}

func NewSPVService(clientId uint64, seeds []string) (SPVService) {
	return &SPVServiceImpl{clientId: clientId, seeds: seeds}
}
