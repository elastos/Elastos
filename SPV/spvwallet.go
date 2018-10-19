package main

import (
	"bytes"
	"encoding/hex"
	"fmt"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/headers"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/sqlite"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
	httputil "github.com/elastos/Elastos.ELA.Utility/http/util"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	MaxPeers        = 12
	MinPeersForSync = 2
)

var ErrInvalidParameter = fmt.Errorf("invalide parameter")

type spvwallet struct {
	sdk.IService
	db     sqlite.DataStore
	filter *sdk.AddrFilter
}

// Batch returns a TxBatch instance for transactions batch
// commit, this can get better performance when commit a bunch
// of transactions within a block.
func (w *spvwallet) Batch() database.TxBatch {
	return &txBatch{
		db:     w.db,
		batch:  w.db.Batch(),
		filter: w.getAddrFilter(),
	}
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (w *spvwallet) HaveTx(txId *common.Uint256) (bool, error) {
	tx, err := w.db.Txs().Get(txId)
	return tx != nil, err
}

// GetTxs returns all transactions within the given height.
func (w *spvwallet) GetTxs(height uint32) ([]*util.Tx, error) {
	return nil, nil
}

// RemoveTxs delete all transactions on the given height.  Return
// how many transactions are deleted from database.
func (w *spvwallet) RemoveTxs(height uint32) (int, error) {
	batch := w.db.Batch()
	err := batch.RollbackHeight(height)
	if err != nil {
		return 0, batch.Rollback()
	}
	return 0, batch.Commit()
}

// Clear delete all data in database.
func (w *spvwallet) Clear() error {
	return w.db.Clear()
}

// Close database.
func (w *spvwallet) Close() error {
	return w.db.Close()
}

func (w *spvwallet) GetFilterData() ([]*common.Uint168, []*util.OutPoint) {
	utxos, err := w.db.UTXOs().GetAll()
	if err != nil {
		waltlog.Debugf("GetAll UTXOs error: %v", err)
	}
	stxos, err := w.db.STXOs().GetAll()
	if err != nil {
		waltlog.Debugf("GetAll STXOs error: %v", err)
	}
	outpoints := make([]*util.OutPoint, 0, len(utxos)+len(stxos))
	for _, utxo := range utxos {
		outpoints = append(outpoints, utxo.Op)
	}
	for _, stxo := range stxos {
		outpoints = append(outpoints, stxo.Op)
	}

	return w.getAddrFilter().GetAddrs(), outpoints
}

func (w *spvwallet) NotifyNewAddress(hash []byte) {
	// Reload address filter to include new address
	w.loadAddrFilter()
	// Broadcast filterload message to connected peers
	w.UpdateFilter()
}

func (w *spvwallet) getAddrFilter() *sdk.AddrFilter {
	if w.filter == nil {
		w.loadAddrFilter()
	}
	return w.filter
}

func (w *spvwallet) loadAddrFilter() *sdk.AddrFilter {
	addrs, _ := w.db.Addrs().GetAll()
	w.filter = sdk.NewAddrFilter(nil)
	for _, addr := range addrs {
		w.filter.AddAddr(addr.Hash())
	}
	return w.filter
}

// TransactionAnnounce will be invoked when received a new announced transaction.
func (w *spvwallet) TransactionAnnounce(tx util.Transaction) {
	// TODO
}

// TransactionAccepted will be invoked after a transaction sent by
// SendTransaction() method has been accepted.  Notice: this method needs at
// lest two connected peers to work.
func (w *spvwallet) TransactionAccepted(tx util.Transaction) {
	// TODO
}

// TransactionRejected will be invoked if a transaction sent by SendTransaction()
// method has been rejected.
func (w *spvwallet) TransactionRejected(tx util.Transaction) {
	// TODO
}

// TransactionConfirmed will be invoked after a transaction sent by
// SendTransaction() method has been packed into a block.
func (w *spvwallet) TransactionConfirmed(tx *util.Tx) {
	// TODO
}

// BlockCommitted will be invoked when a block and transactions within it are
// successfully committed into database.
func (w *spvwallet) BlockCommitted(block *util.Block) {
	if !w.IsCurrent() {
		return
	}

	w.db.State().PutHeight(block.Height)
	// TODO
}

type txBatch struct {
	db     sqlite.DataStore
	batch  sqlite.DataBatch
	filter *sdk.AddrFilter
}

// PutTx add a store transaction operation into batch, and return
// if it is a false positive and error.
func (b *txBatch) PutTx(mtx util.Transaction, height uint32) (bool, error) {
	tx := mtx.(*core.Transaction)
	txId := tx.Hash()
	hits := 0

	// Check if any UTXOs within this wallet have been spent.
	for _, input := range tx.Inputs {
		// Move UTXO to STXO
		op := util.NewOutPoint(input.Previous.TxID, input.Previous.Index)
		utxo, _ := b.db.UTXOs().Get(op)
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
			var lockTime = output.OutputLock
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
	err := b.batch.Txs().Put(util.NewTx(tx, height))
	if err != nil {
		return false, err
	}

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

// Functions for RPC service.
func (w *spvwallet) notifyNewAddress(params httputil.Params) (interface{}, error) {
	data, ok := params.String("addr")
	if !ok {
		return nil, ErrInvalidParameter
	}

	_, err := hex.DecodeString(data)
	if err != nil {
		return nil, err
	}

	// Reload address filter to include new address
	w.loadAddrFilter()

	// Broadcast filterload message to connected peers
	w.UpdateFilter()

	return nil, nil
}

func (w *spvwallet) sendTransaction(params httputil.Params) (interface{}, error) {
	data, ok := params.String("data")
	if !ok {
		return nil, ErrInvalidParameter
	}

	txBytes, err := hex.DecodeString(data)
	if err != nil {
		return nil, ErrInvalidParameter
	}

	var tx core.Transaction
	err = tx.Deserialize(bytes.NewReader(txBytes))
	if err != nil {
		return nil, fmt.Errorf("deserialize transaction failed %s", err)
	}

	return nil, w.SendTransaction(&tx)
}

func NewWallet() (*spvwallet, error) {
	// Initialize headers db
	headers, err := headers.NewDatabase(newBlockHeader)
	if err != nil {
		return nil, err
	}

	db, err := sqlite.NewDatabase()
	if err != nil {
		return nil, err
	}

	w := spvwallet{
		db: db,
	}
	chainStore := database.NewDefaultChainDB(headers, &w)

	// Initialize spv service
	w.IService, err = sdk.NewService(
		&sdk.Config{
			Magic:           config.Magic,
			SeedList:        config.SeedList,
			DefaultPort:     config.NodePort,
			MaxPeers:        MaxPeers,
			MinPeersForSync: MinPeersForSync,
			GenesisHeader:   GenesisHeader(),
			ChainStore:      chainStore,
			NewTransaction:  newTransaction,
			NewBlockHeader:  newBlockHeader,
			GetFilterData:   w.GetFilterData,
			StateNotifier:   &w,
		})
	if err != nil {
		return nil, err
	}

	return &w, nil
}

func newTransaction() util.Transaction {
	return new(core.Transaction)
}

// GenesisHeader creates a specific genesis header by the given
// foundation address.
func GenesisHeader() util.BlockHeader {
	// Genesis time
	genesisTime := time.Date(2017, time.December, 22, 10, 0, 0, 0, time.UTC)

	// header
	header := core.Header{
		Version:    core.BlockVersion,
		Previous:   common.EmptyHash,
		MerkleRoot: common.EmptyHash,
		Timestamp:  uint32(genesisTime.Unix()),
		Bits:       0x1d03ffff,
		Nonce:      core.GenesisNonce,
		Height:     uint32(0),
	}

	// ELA coin
	elaCoin := &core.Transaction{
		TxType:         core.RegisterAsset,
		PayloadVersion: 0,
		Payload: &core.PayloadRegisterAsset{
			Asset: core.Asset{
				Name:      "ELA",
				Precision: 0x08,
				AssetType: 0x00,
			},
			Amount:     0 * 100000000,
			Controller: common.Uint168{},
		},
		Attributes: []*core.Attribute{},
		Inputs:     []*core.Input{},
		Outputs:    []*core.Output{},
		Programs:   []*core.Program{},
	}

	coinBase := &core.Transaction{
		TxType:         core.CoinBase,
		PayloadVersion: core.PayloadCoinBaseVersion,
		Payload:        new(core.PayloadCoinBase),
		Inputs: []*core.Input{
			{
				Previous: core.OutPoint{
					TxID:  common.EmptyHash,
					Index: 0x0000,
				},
				Sequence: 0x00000000,
			},
		},
		Attributes: []*core.Attribute{},
		LockTime:   0,
		Programs:   []*core.Program{},
	}

	coinBase.Outputs = []*core.Output{
		{
			AssetID:     elaCoin.Hash(),
			Value:       3300 * 10000 * 100000000,
			ProgramHash: *config.foundation,
		},
	}

	nonce := []byte{0x4d, 0x65, 0x82, 0x21, 0x07, 0xfc, 0xfd, 0x52}
	txAttr := core.NewAttribute(core.Nonce, nonce)
	coinBase.Attributes = append(coinBase.Attributes, &txAttr)

	transactions := []*core.Transaction{coinBase, elaCoin}
	hashes := make([]common.Uint256, 0, len(transactions))
	for _, tx := range transactions {
		hashes = append(hashes, tx.Hash())
	}
	header.MerkleRoot, _ = crypto.ComputeRoot(hashes)

	return &blockHeader{Header: &header}
}
