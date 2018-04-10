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

	// Register the TransactionListener to receive transaction notifications
	// when a transaction related with the registered accounts is received
	RegisterTransactionListener(TransactionListener)

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

/*
Register this listener into the SPVService RegisterTransactionListener() method
to receive transaction notifications.
*/
type TransactionListener interface {
	// Type() indicates which transaction type this listener are interested
	Type() tx.TransactionType

	// Confirmed() indicates if this transaction should be callback after reach the confirmed height,
	// by default 6 confirmations are needed according to the protocol
	Confirmed() bool

	// Notify() is the method to callback the received transaction
	// with the merkle tree proof to verify it
	Notify(db.Proof, tx.Transaction)
}

func NewSPVService(clientId uint64, seeds []string) (SPVService) {
	return newSPVServiceImpl(clientId, seeds)
}
