package _interface

import (
	"errors"
	"os"
	"os/signal"

	"github.com/elastos/Elastos.ELA.SPV/interface/db"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
)

type SPVServiceImpl struct {
	sdk.SPVService
	headers   *db.HeaderStore
	dataStore db.DataStore
	queue     db.Queue
	listeners map[core.TransactionType][]TransactionListener
}

func NewSPVServiceImpl(magic uint32, clientId uint64, seeds []string) (*SPVServiceImpl, error) {
	var err error
	service := new(SPVServiceImpl)
	service.headers, err = db.NewHeaderStore()
	if err != nil {
		return nil, err
	}

	service.dataStore, err = db.NewDataStore()
	if err != nil {
		return nil, err
	}

	service.queue, err = db.NewQueueDB()
	if err != nil {
		return nil, err
	}

	spvClient, err := sdk.GetSPVClient(magic, clientId, seeds)
	if err != nil {
		return nil, err
	}

	service.SPVService, err = sdk.GetSPVService(spvClient, service.headers, service)
	if err != nil {
		return nil, err
	}

	service.listeners = make(map[core.TransactionType][]TransactionListener)

	return service, nil
}

func (service *SPVServiceImpl) RegisterAccount(address string) error {
	ok, err := service.dataStore.Addrs().Put(address)
	if err != nil {
		return err
	}
	if !ok {
		return errors.New("address has already registered")
	}
	service.SPVService.ReloadFilter()
	return nil
}

func (service *SPVServiceImpl) RegisterTransactionListener(listener TransactionListener) {
	listeners := service.listeners[listener.Type()]
	listeners = append(listeners, listener)
	service.listeners[listener.Type()] = listeners
}

func (service *SPVServiceImpl) SubmitTransactionReceipt(txHash common.Uint256) error {
	return service.queue.Delete(&txHash)
}

func (service *SPVServiceImpl) VerifyTransaction(proof bloom.MerkleProof, tx core.Transaction) error {
	// Get Header from main chain
	header, err := service.headers.GetHeader(&proof.BlockHash)
	if err != nil {
		return errors.New("can not get block from main chain")
	}

	// Check if merkleroot is match
	merkleBlock := msg.MerkleBlock{
		Header:       &header.Header,
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
		if *txId == tx.Hash() {
			match = true
			break
		}
	}
	if !match {
		return errors.New("transaction hash not match proof")
	}

	return nil
}

func (service *SPVServiceImpl) SendTransaction(tx core.Transaction) error {
	_, err := service.SPVService.SendTransaction(tx)
	return err
}

func (service *SPVServiceImpl) HeaderStore() store.HeaderStore {
	return service.headers
}

func (service *SPVServiceImpl) GetData() ([]*common.Uint168, []*core.OutPoint) {
	ops, err := service.dataStore.Outpoints().GetAll()
	if err != nil {
		log.Error("[SPV_SERVICE] GetData error ", err)
	}

	return service.dataStore.Addrs().GetAll(), ops
}

func (service *SPVServiceImpl) OnStateChange(sdk.ChainState) {}

func (service *SPVServiceImpl) CommitTx(tx *core.Transaction, height uint32) (bool, error) {
	hits := 0
	for index, output := range tx.Outputs {
		if service.dataStore.Addrs().GetFilter().ContainAddr(output.ProgramHash) {
			outpoint := core.NewOutPoint(tx.Hash(), uint16(index))
			if err := service.dataStore.Outpoints().Put(outpoint); err != nil {
				return false, err
			}
			hits++
		}
	}

	for _, input := range tx.Inputs {
		if service.dataStore.Outpoints().IsExist(&input.Previous) {
			hits++
		}
	}

	if hits == 0 {
		return true, nil
	}

	return false, service.dataStore.Txs().Put(db.NewStoreTx(tx, height))
}

func (service *SPVServiceImpl) OnBlockCommitted(block *msg.MerkleBlock, txs []*core.Transaction) {
	header := block.Header.(*core.Header)

	// Store merkle proof
	err := service.dataStore.Proofs().Put(&bloom.MerkleProof{
		BlockHash:    header.Hash(),
		Height:       header.Height,
		Transactions: block.Transactions,
		Hashes:       block.Hashes,
		Flags:        block.Flags,
	})

	if err != nil {
		log.Errorf("[SPV_SERVICE] store merkle proof failed, error %s", err.Error())
		return
	}

	// Find transactions matches registered accounts
	var matchedTxs []*core.Transaction
	for _, tx := range txs {
		for _, output := range tx.Outputs {
			if service.dataStore.Addrs().GetFilter().ContainAddr(output.ProgramHash) {
				matchedTxs = append(matchedTxs, tx)
			}
		}
	}

	// Queue matched transactions
	for _, tx := range matchedTxs {
		item := &db.QueueItem{
			TxHash:    tx.Hash(),
			BlockHash: header.Hash(),
			Height:    header.Height,
		}

		// Save to queue db
		service.queue.Put(item)
	}

	// Look up for queued transactions
	items, err := service.queue.GetAll()
	if err != nil {
		return
	}
	for _, item := range items {
		//	Get proof from db
		proof, err := service.dataStore.Proofs().Get(&item.BlockHash)
		if err != nil {
			log.Error("Query merkle proof failed, block hash:", item.BlockHash.String())
			return
		}
		//	Get transaction from db
		storeTx, err := service.dataStore.Txs().Get(&item.TxHash)
		if err != nil {
			log.Error("Query transaction failed, tx hash:", item.TxHash.String())
			return
		}
		// Prune the proof by the given transaction id
		proof = getTransactionProof(proof, storeTx.Hash())

		// Notify listeners
		service.notifyTransaction(*proof, storeTx.Transaction, header.Height-item.Height)
	}
}

// Overwrite OnRollback() method in SPVWallet
func (service *SPVServiceImpl) OnRollback(height uint32) error {
	err := service.dataStore.Rollback(height)
	if err != nil {
		log.Warnf("Rollback data store error %s", err.Error())
	}
	err = service.queue.Rollback(height)
	if err != nil {
		log.Warnf("Rollback transaction notify queue error %s", err.Error())
	}
	service.notifyRollback(height)
	return nil
}

func (service *SPVServiceImpl) Start() error {
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
	service.SPVService.Start()

	<-stop

	return nil
}

func (service *SPVServiceImpl) ResetStores() error {
	err := service.headers.Reset()
	if err != nil {
		log.Warnf("Reset header store error %s", err.Error())
	}
	err = service.dataStore.Reset()
	if err != nil {
		log.Warnf("Reset data store error %s", err.Error())
	}
	err = service.queue.Reset()
	if err != nil {
		log.Warnf("Reset transaction notify queue store error %s", err.Error())
	}
	return nil
}

func (service *SPVServiceImpl) notifyTransaction(proof bloom.MerkleProof, tx core.Transaction, confirmations uint32) {
	listeners := service.listeners[tx.TxType]
	for _, listener := range listeners {
		if listener.Confirmed() {
			if confirmations >= getConfirmations(tx) {
				go listener.Notify(proof, tx)
			}
		} else {
			go listener.Notify(proof, tx)
		}
	}
}

func (service *SPVServiceImpl) notifyRollback(height uint32) {
	for _, group := range service.listeners {
		for _, listener := range group {
			go listener.Rollback(height)
		}
	}
}

func getConfirmations(tx core.Transaction) uint32 {
	// TODO user can set confirmations attribute in transaction,
	// if the confirmation attribute is set, use it instead of default value
	if tx.TxType == core.CoinBase {
		return 100
	}
	return DefaultConfirmations
}

func getTransactionProof(proof *bloom.MerkleProof, txHash common.Uint256) *bloom.MerkleProof {
	// TODO Pick out the merkle proof of the transaction
	return proof
}
