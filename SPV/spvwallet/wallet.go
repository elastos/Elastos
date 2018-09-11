package spvwallet

import (
	"time"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/store/headers"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/store/sqlite"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/sutil"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	MaxUnconfirmedTime = time.Minute * 30
	MaxTxIdCached      = 1000
	MaxPeers           = 12
	MinPeersForSync    = 2
)

type Wallet struct {
	sdk.IService
	rpcServer  *rpc.Server
	chainStore database.ChainStore
	db         sqlite.DataStore
	txIds      *TxIdCache
	filter     *sdk.AddrFilter
}

func (w *Wallet) Start() {
	w.IService.Start()
	w.rpcServer.Start()
}

func (w *Wallet) Stop() {
	w.IService.Stop()
	w.rpcServer.Close()
}

// Batch returns a TxBatch instance for transactions batch
// commit, this can get better performance when commit a bunch
// of transactions within a block.
func (w *Wallet) Batch() database.TxBatch {
	return &txBatch{
		db:     w.db,
		batch:  w.db.Batch(),
		ids:    w.txIds,
		filter: w.getAddrFilter(),
	}
}

// CommitTx save a transaction into database, and return
// if it is a false positive and error.
func (w *Wallet) CommitTx(tx *util.Tx) (bool, error) {
	// In this SPV implementation, CommitTx only invoked on new transactions
	// that are unconfirmed, we just check if this transaction is a false
	// positive and store it into database. NOTICE: at this moment, we didn't
	// change UTXOs and STXOs.
	txId := tx.Hash()

	// We already have this transaction.
	ok := w.txIds.Get(txId)
	if ok {
		return false, nil
	}

	hits := 0
	// Check if any UTXOs of this wallet are used.
	for _, input := range tx.Inputs {
		stxo, _ := w.db.UTXOs().Get(&input.Previous)
		if stxo != nil {
			hits++
		}
	}

	// Check if there are any output to this wallet address.
	for _, output := range tx.Outputs {
		if w.getAddrFilter().ContainAddr(output.ProgramHash) {
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction as unconfirmed.
	err := w.db.Txs().Put(sutil.NewTx(tx.Transaction, 0))
	if err != nil {
		return false, err
	}

	w.txIds.Add(txId)

	return false, nil
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (w *Wallet) HaveTx(txId *common.Uint256) (bool, error) {
	tx, err := w.db.Txs().Get(txId)
	return tx != nil, err
}

// GetTxs returns all transactions within the given height.
func (w *Wallet) GetTxs(height uint32) ([]*util.Tx, error) {
	return nil, nil
}

// RemoveTxs delete all transactions on the given height.  Return
// how many transactions are deleted from database.
func (w *Wallet) RemoveTxs(height uint32) (int, error) {
	batch := w.db.Batch()
	err := batch.RollbackHeight(height)
	if err != nil {
		return 0, batch.Rollback()
	}
	return 0, batch.Commit()
}

// Clear delete all data in database.
func (w *Wallet) Clear() error {
	return w.db.Clear()
}

// Close database.
func (w *Wallet) Close() error {
	return w.db.Close()
}

func (w *Wallet) GetFilterData() ([]*common.Uint168, []*core.OutPoint) {
	utxos, err := w.db.UTXOs().GetAll()
	if err != nil {
		log.Debugf("GetAll UTXOs error: %v", err)
	}
	stxos, err := w.db.STXOs().GetAll()
	if err != nil {
		log.Debugf("GetAll STXOs error: %v", err)
	}
	outpoints := make([]*core.OutPoint, 0, len(utxos)+len(stxos))
	for _, utxo := range utxos {
		outpoints = append(outpoints, &utxo.Op)
	}
	for _, stxo := range stxos {
		outpoints = append(outpoints, &stxo.Op)
	}

	return w.getAddrFilter().GetAddrs(), outpoints
}

func (w *Wallet) NotifyNewAddress(hash []byte) {
	// Reload address filter to include new address
	w.loadAddrFilter()
	// Broadcast filterload message to connected peers
	w.UpdateFilter()
}

func (w *Wallet) getAddrFilter() *sdk.AddrFilter {
	if w.filter == nil {
		w.loadAddrFilter()
	}
	return w.filter
}

func (w *Wallet) loadAddrFilter() *sdk.AddrFilter {
	addrs, _ := w.db.Addrs().GetAll()
	w.filter = sdk.NewAddrFilter(nil)
	for _, addr := range addrs {
		w.filter.AddAddr(addr.Hash())
	}
	return w.filter
}

// TransactionAccepted will be invoked after a transaction sent by
// SendTransaction() method has been accepted.  Notice: this method needs at
// lest two connected peers to work.
func (w *Wallet) TransactionAccepted(tx *util.Tx) {
	// TODO
}

// TransactionRejected will be invoked if a transaction sent by SendTransaction()
// method has been rejected.
func (w *Wallet) TransactionRejected(tx *util.Tx) {
	// TODO

}

// TransactionConfirmed will be invoked after a transaction sent by
// SendTransaction() method has been packed into a block.
func (w *Wallet) TransactionConfirmed(tx *util.Tx) {
	// TODO

}

// BlockCommitted will be invoked when a block and transactions within it are
// successfully committed into database.
func (w *Wallet) BlockCommitted(block *util.Block) {
	if w.IsCurrent() {
		w.db.State().PutHeight(block.Height)
		// Get all unconfirmed transactions
		txs, err := w.db.Txs().GetAllUnconfirmed()
		if err != nil {
			log.Debugf("Get unconfirmed transactions failed, error %s", err.Error())
			return
		}
		now := time.Now()
		for _, tx := range txs {
			if now.After(tx.Timestamp.Add(MaxUnconfirmedTime)) {
				err = w.db.Txs().Del(&tx.TxId)
				if err != nil {
					log.Errorf("Delete timeout transaction %s failed, error %s", tx.TxId.String(), err.Error())
				}
			}
		}
	}
}

type txBatch struct {
	db     sqlite.DataStore
	batch  sqlite.DataBatch
	ids    *TxIdCache
	filter *sdk.AddrFilter
}

// AddTx add a store transaction operation into batch, and return
// if it is a false positive and error.
func (b *txBatch) AddTx(tx *util.Tx) (bool, error) {
	// This AddTx in batch used by the ChainStore when storing a block.
	// That means this transaction has been confirmed, so we need to remove
	// it from unconfirmed list. And also, double spend transactions should
	// been removed from unconfirmed list as well.
	txId := tx.Hash()
	height := tx.Height
	dubs, err := b.checkDoubleSpends(tx)
	if err != nil {
		return false, nil
	}
	// Delete any double spend transactions
	if len(dubs) > 0 {
		batch := b.db.Txs().Batch()
		for _, dub := range dubs {
			if err := batch.Del(dub); err != nil {
				batch.Rollback()
				return false, nil
			}
		}
		batch.Commit()
	}

	hits := 0
	// Check if any UTXOs within this wallet have been spent.
	for _, input := range tx.Inputs {
		// Move UTXO to STXO
		utxo, _ := b.db.UTXOs().Get(&input.Previous)
		// Skip if no match.
		if utxo == nil {
			continue
		}

		err := b.batch.STXOs().Put(sutil.NewSTXO(utxo, height, txId))
		if err != nil {
			return false, nil
		}
		hits++
	}

	// Check if there are any output to this wallet address.
	for index, output := range tx.Outputs {
		// Filter address
		if b.filter.ContainAddr(output.ProgramHash) {
			var lockTime uint32
			if tx.TxType == core.CoinBase {
				lockTime = height + 100
			}
			utxo := sutil.NewUTXO(txId, height, index, output.Value, lockTime, output.ProgramHash)
			err := b.batch.UTXOs().Put(utxo)
			if err != nil {
				return false, err
			}
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction
	err = b.batch.Txs().Put(sutil.NewTx(tx.Transaction, height))
	if err != nil {
		return false, err
	}

	b.ids.Add(txId)

	return false, nil
}

// DelTx add a delete transaction operation into batch.
func (b *txBatch) DelTx(txId *common.Uint256) error {
	return b.batch.Txs().Del(txId)
}

// DelTxs add a delete transactions on given height operation.
func (b *txBatch) DelTxs(height uint32) error {
	// Delete transactions is used when blockchain doing rollback, this not
	// only delete the transactions on the given height, and also restore
	// STXOs and remove UTXOs within these transactions.
	return b.batch.RollbackHeight(height)
}

// Rollback cancel all operations in current batch.
func (b *txBatch) Rollback() error {
	return b.batch.Rollback()
}

// Commit the added transactions into database.
func (b *txBatch) Commit() error {
	return b.batch.Commit()
}

// checkDoubleSpends takes a transaction and compares it with all unconfirmed
// transactions in the db.  It returns a slice of txIds in the db  which are
// double spent by the received tx.
func (b *txBatch) checkDoubleSpends(tx *util.Tx) ([]*common.Uint256, error) {
	txId := tx.Hash()
	txs, err := b.db.Txs().GetAllUnconfirmed()
	if err != nil {
		return nil, err
	}

	inputs := make(map[string]*common.Uint256)
	for _, compTx := range txs {
		// Skip coinbase transaction
		if compTx.Data.IsCoinBaseTx() {
			continue
		}

		// Skip duplicate transaction
		compTxId := compTx.Data.Hash()
		if compTxId.IsEqual(txId) {
			continue
		}

		for _, in := range compTx.Data.Inputs {
			inputs[in.ReferKey()] = &compTxId
		}
	}

	var dubs []*common.Uint256
	for _, in := range tx.Inputs {
		if tx, ok := inputs[in.ReferKey()]; ok {
			dubs = append(dubs, tx)
		}
	}
	return dubs, nil
}

func New() (*Wallet, error) {
	wallet := new(Wallet)

	// Initialize headers db
	headers, err := headers.New()
	if err != nil {
		return nil, err
	}

	// Initialize singleton database
	wallet.db, err = sqlite.New()
	if err != nil {
		return nil, err
	}

	// Initiate ChainStore
	wallet.chainStore = database.NewDefaultChainDB(headers, wallet)

	// Initialize txs cache
	wallet.txIds = NewTxIdCache(MaxTxIdCached)

	// Initialize spv service
	wallet.IService, err = sdk.NewService(
		&sdk.Config{
			Magic:           config.Values().Magic,
			SeedList:        config.Values().SeedList,
			DefaultPort:     config.Values().DefaultPort,
			MaxPeers:        MaxPeers,
			MinPeersForSync: MinPeersForSync,
			Foundation:      config.Values().Foundation,
			ChainStore:      wallet.chainStore,
			GetFilterData:   wallet.GetFilterData,
			StateNotifier:   wallet,
		})
	if err != nil {
		return nil, err
	}

	// Initialize RPC server
	server := rpc.InitServer()
	server.NotifyNewAddress = wallet.NotifyNewAddress
	server.SendTransaction = wallet.IService.SendTransaction
	wallet.rpcServer = server

	return wallet, nil
}
