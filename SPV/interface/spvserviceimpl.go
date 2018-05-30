package _interface

import (
	"errors"
	"os"
	"os/signal"

	"github.com/elastos/Elastos.ELA.SPV/interface/db"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/store"

	"fmt"
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
	listeners map[common.Uint168]TransactionListener
}

func NewSPVServiceImpl(magic uint32, clientId uint64, seeds []string, minOutbound, maxConnections int) (*SPVServiceImpl, error) {
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

	spvClient, err := sdk.GetSPVClient(magic, clientId, seeds, minOutbound, maxConnections)
	if err != nil {
		return nil, err
	}

	service.SPVService, err = sdk.GetSPVService(spvClient, service.headers, service)
	if err != nil {
		return nil, err
	}

	service.listeners = make(map[common.Uint168]TransactionListener)

	return service, nil
}

func (service *SPVServiceImpl) RegisterTransactionListener(listener TransactionListener) error {
	address, err := common.Uint168FromAddress(listener.Address())
	if err != nil {
		return fmt.Errorf("address %s is not a valied address", listener.Address())
	}
	if _, ok := service.listeners[*address]; ok {
		return fmt.Errorf("address %s already registered", listener.Address())
	}
	service.listeners[*address] = listener
	return service.dataStore.Addrs().Put(address)
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
		return fmt.Errorf("check merkle branch failed, %s", err.Error())
	}
	if len(txIds) == 0 {
		return fmt.Errorf("invalid transaction proof, no transactions found")
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
		return fmt.Errorf("transaction hash not match proof")
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

func (service *SPVServiceImpl) CommitTx(tx *core.Transaction, height uint32) (bool, error) {
	hits := 0
	for index, output := range tx.Outputs {
		if service.dataStore.Addrs().GetFilter().ContainAddr(output.ProgramHash) {
			outpoint := core.NewOutPoint(tx.Hash(), uint16(index))
			if err := service.dataStore.Outpoints().Put(outpoint, output.ProgramHash); err != nil {
				return false, err
			}
			hits++
		}
	}

	for _, input := range tx.Inputs {
		if addr := service.dataStore.Outpoints().IsExist(&input.Previous); addr != nil {
			hits++
		}
	}

	if hits == 0 {
		return true, nil
	}

	// Put hit transaction into notify queue
	service.queue.Put(&db.QueueItem{
		TxHash: tx.Hash(),
		Height: height,
	})

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

	// Look up for queued transactions
	items, err := service.queue.GetAll()
	if err != nil {
		return
	}
	for _, item := range items {
		//	Get proof from db
		proof, err := service.dataStore.Proofs().Get(item.Height)
		if err != nil {
			log.Error("Query merkle proof at height %d failed, %s", item.Height, err.Error())
			return
		}
		//	Get transaction from db
		storeTx, err := service.dataStore.Txs().Get(&item.TxHash)
		if err != nil {
			log.Error("Query transaction failed, tx hash:", item.TxHash.String())
			return
		}

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
	// common function
	notifyListener := func(listener TransactionListener) {
		txId := tx.Hash()

		// Remove notifications that not match the listener's type
		if listener.Type() != tx.TxType {
			service.queue.Delete(&txId)
			return
		}

		// Remove notifications if FlagNotifyInSyncing not set
		if service.SPVService.ChainState() == sdk.SYNCING &&
			listener.Flags()&FlagNotifyInSyncing != FlagNotifyInSyncing {

			if listener.Flags()&FlagNotifyConfirmed == FlagNotifyConfirmed {
				if confirmations >= getConfirmations(tx) {
					service.queue.Delete(&txId)
				}
			} else {
				service.queue.Delete(&txId)
			}
			return
		}

		// Notify listener
		if listener.Flags()&FlagNotifyConfirmed == FlagNotifyConfirmed {
			if confirmations >= getConfirmations(tx) {
				listener.Notify(proof, tx)
			}
		} else {
			listener.Notify(proof, tx)
		}
	}

	for _, output := range tx.Outputs {
		if listener, ok := service.listeners[output.ProgramHash]; ok {
			go notifyListener(listener)
		}
	}

	for _, input := range tx.Inputs {
		if addr := service.dataStore.Outpoints().IsExist(&input.Previous); addr != nil {
			if listener, ok := service.listeners[*addr]; ok {
				go notifyListener(listener)
			}
		}
	}
}

func (service *SPVServiceImpl) notifyRollback(height uint32) {
	for _, listener := range service.listeners {
		go listener.Rollback(height)
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
