package _interface

import (
	"os"
	"errors"
	"os/signal"

	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
)

type SPVServiceImpl struct {
	*spvwallet.SPVWallet
	clientId   uint64
	seeds      []string
	accounts   []*Uint168
	addrFilter *sdk.AddrFilter
	listeners  map[tx.TransactionType][]TransactionListener
}

func newSPVServiceImpl(clientId uint64, seeds []string) *SPVServiceImpl {
	return &SPVServiceImpl{
		clientId:  clientId,
		seeds:     seeds,
		listeners: make(map[tx.TransactionType][]TransactionListener),
	}
}

func (service *SPVServiceImpl) RegisterAccount(address string) error {
	account, err := Uint168FromAddress(address)
	if err != nil {
		return errors.New("Invalid address format")
	}
	service.accounts = append(service.accounts, account)
	return nil
}

func (service *SPVServiceImpl) RegisterTransactionListener(listener TransactionListener) {
	listeners := service.listeners[listener.Type()]
	listeners = append(listeners, listener)
	service.listeners[listener.Type()] = listeners
	log.Debug("Listener registered:", listeners)
}

func (service *SPVServiceImpl) SubmitTransactionReceipt(txHash Uint256) error {
	return service.DataStore().Queue().Delete(&txHash)
}

func (service *SPVServiceImpl) VerifyTransaction(proof db.Proof, tx tx.Transaction) error {
	if service.SPVWallet == nil {
		return errors.New("SPV service not started")
	}

	// Get Header from main chain
	header, err := service.Headers().GetHeader(proof.BlockHash)
	if err != nil {
		return errors.New("can not get block from main chain")
	}

	// Check if merkleroot is match
	merkleBlock := bloom.MerkleBlock{
		BlockHeader:  header.Header,
		Transactions: proof.Transactions,
		Hashes:       proof.Hashes,
		Flags:        proof.Flags,
	}
	txIds, err := bloom.CheckMerkleBlock(merkleBlock)
	if err != nil {
		return errors.New("check merkle branch failed, " + err.Error())
	}
	if len(txIds) == 0 {
		return errors.New("invalid transaction proof, no transactions found")
	}

	// Check if transaction hash is match
	match := false
	for _, txId := range txIds {
		if *txId == *tx.Hash() {
			match = true
			break
		}
	}
	if !match {
		return errors.New("transaction hash not match proof")
	}

	return nil
}

func (service *SPVServiceImpl) SendTransaction(tx tx.Transaction) error {
	if service.SPVWallet == nil {
		return errors.New("SPV service not started")
	}

	return service.SPVWallet.SendTransaction(tx)
}

func (service *SPVServiceImpl) Start() error {
	if service.SPVWallet != nil {
		return errors.New("SPV service already started")
	}

	var err error
	service.SPVWallet, err = spvwallet.Init(service.clientId, service.seeds)
	if err != nil {
		return err
	}

	// Register accounts
	if len(service.accounts) == 0 {
		return errors.New("No account registered")
	}
	for _, account := range service.accounts {
		service.DataStore().Addrs().Put(account, RegisteredAccountScript, db.TypeNotify)
	}

	// Create address filter by accounts
	service.addrFilter = sdk.NewAddrFilter(service.accounts)

	// Set callback
	service.SPVWallet.Blockchain().AddStateListener(service)

	// Handle interrupt signal
	stop := make(chan int, 1)
	signals := make(chan os.Signal, 1)
	signal.Notify(signals, os.Interrupt)
	go func() {
		for range signals {
			log.Trace("SPV service shutting down...")
			service.Stop()
			stop <- 1
		}
	}()

	// Start SPV service
	service.SPVWallet.Start()

	<-stop

	return nil
}

func (service *SPVServiceImpl) OnBlockCommitted(block bloom.MerkleBlock, txs []tx.Transaction) {
	header := block.BlockHeader
	// If no transactions return
	if len(txs) == 0 {
		return
	}

	// Find transactions matches registered accounts
	var matchedTxs []tx.Transaction
	for _, tx := range txs {
		for _, output := range tx.Outputs {
			if service.addrFilter.ContainAddr(output.ProgramHash) {
				matchedTxs = append(matchedTxs, tx)
			}
		}
	}

	// Queue matched transactions
	for _, tx := range matchedTxs {
		item := &db.QueueItem{
			TxHash:    *tx.Hash(),
			BlockHash: *header.Hash(),
			Height:    header.Height,
		}

		// Save to queue db
		service.DataStore().Queue().Put(item)
	}

	// Look up for queued transactions
	items, err := service.DataStore().Queue().GetAll()
	if err != nil {
		return
	}
	for _, item := range items {
		//	Get proof from db
		proof, err := service.Proofs().Get(&item.BlockHash)
		if err != nil {
			log.Error("Query merkle proof failed, block hash:", item.BlockHash.String())
			return
		}
		//	Get transaction from db
		storeTx, err := service.DataStore().Txs().Get(&item.TxHash)
		if err != nil {
			log.Error("Query transaction failed, tx hash:", item.TxHash.String())
			return
		}
		// Prune the proof by the given transaction id
		proof = getTransactionProof(proof, storeTx.TxId)

		// Notify listeners
		service.notifyListeners(proof, storeTx.Data, header.Height-item.Height)
	}
}

func (service *SPVServiceImpl) notifyListeners(proof *db.Proof, tx tx.Transaction, confirmations uint32) {
	listeners := service.listeners[tx.TxType]
	for _, listener := range listeners {
		if listener.Confirmed() {
			if confirmations >= getConfirmations(tx) {
				listener.Notify(*proof, tx)
			}
		} else {
			listener.Notify(*proof, tx)
		}
	}
}

func getConfirmations(tx tx.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	return DefaultConfirmations
}

func getTransactionProof(proof *db.Proof, txHash Uint256) *db.Proof {
	// TODO Pick out the merkle proof of the transaction
	return proof
}
