package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
)

/*
SPV service is the interface to interactive with the SPV (Simplified Payment Verification)
service implementation running background, you can register specific accounts that you are
interested in and receive transaction notifications of these accounts.
*/
type SPVService interface {
	// RegisterTransactionListener register the listener to receive transaction notifications
	// listeners must be registered before call Start() method, or some notifications will go missing.
	RegisterTransactionListener(TransactionListener) error

	// After receive the transaction callback, call this method
	// to confirm that the transaction with the given ID was handled,
	// so the transaction will be removed from the notify queue.
	// the notifyId is the key to specify which listener received this notify.
	SubmitTransactionReceipt(notifyId common.Uint256, txId common.Uint256) error

	// To verify if a transaction is valid
	// This method is useful when receive a transaction from other peer
	VerifyTransaction(bloom.MerkleProof, core.Transaction) error

	// Send a transaction to the P2P network
	SendTransaction(core.Transaction) error

	// Get headers database
	HeaderStore() store.HeaderStore

	// Start the SPV service
	Start() error

	// ResetStores clear all data stores data including HeaderStore, ProofStore, AddrsStore, TxsStore etc.
	ResetStores() error
}

const (
	// FlagNotifyConfirmed indicates if this transaction should be callback after reach the confirmed height,
	// by default 6 confirmations are needed according to the protocol
	FlagNotifyConfirmed = 1 << 0

	// FlagNotifyInSyncing indicates if notify this listener when SPV is in syncing.
	FlagNotifyInSyncing = 1 << 1
)

/*
Register this listener into the SPVService RegisterTransactionListener() method
to receive transaction notifications.
*/
type TransactionListener interface {
	// The address this listener interested
	Address() string

	// Type() indicates which transaction type this listener are interested
	Type() core.TransactionType

	// Flags control the notification actions by the given flag
	Flags() uint64

	// Notify() is the method to callback the received transaction
	// with the merkle tree proof to verify it, the notifyId is key of this
	// notify message and it must be submitted with the receipt together.
	Notify(notifyId common.Uint256, proof bloom.MerkleProof, tx core.Transaction)

	// Rollback callbacks that, the transactions
	// on the given height has been rollback
	Rollback(height uint32)
}

func NewSPVService(magic uint32, foundation string, clientId uint64, seeds []string, minOutbound, maxConnections int) (SPVService, error) {
	return NewSPVServiceImpl(magic, foundation, clientId, seeds, minOutbound, maxConnections)
}
