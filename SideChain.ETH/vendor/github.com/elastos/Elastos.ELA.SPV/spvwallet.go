package main

import (
	"bytes"
	"encoding/hex"
	"fmt"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/headers"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/sqlite"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/elanet/filter"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/http/jsonrpc"
)

const (
	MaxPeers = 12
)

var ErrInvalidParameter = fmt.Errorf("invalide parameter")

type spvwallet struct {
	sdk.IService
	db     sqlite.DataStore
	filter *sdk.AddrFilter
}

func (w *spvwallet) putTx(batch sqlite.DataBatch, utx util.Transaction,
	height uint32) (bool, error) {

	tx := utx.(*sutil.Tx)
	txId := tx.Hash()
	hits := 0

	// Check if any UTXOs within this wallet have been spent.
	for _, input := range tx.Inputs {
		// Move UTXO to STXO
		op := util.NewOutPoint(input.Previous.TxID, input.Previous.Index)
		utxo, _ := w.db.UTXOs().Get(op)
		// Skip if no match.
		if utxo == nil {
			continue
		}

		err := batch.STXOs().Put(sutil.NewSTXO(utxo, height, txId))
		if err != nil {
			return false, nil
		}
		hits++
	}

	// Check if there are any output to this wallet address.
	for index, output := range tx.Outputs {
		// Filter address
		if w.getAddrFilter().ContainAddr(output.ProgramHash) {
			var lockTime = output.OutputLock
			if tx.TxType == types.CoinBase {
				lockTime = height + 100
			}
			utxo := sutil.NewUTXO(txId, height, index, output.Value, lockTime, output.ProgramHash)
			err := batch.UTXOs().Put(utxo)
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
	err := batch.Txs().Put(util.NewTx(tx, height))
	if err != nil {
		return false, err
	}

	return false, nil
}

// PutTxs persists the main chain transactions into database and can be
// queried by GetTxs(height).  Returns the false positive transaction count
// and error.
func (w *spvwallet) PutTxs(txs []util.Transaction, height uint32) (uint32, error) {
	fps := uint32(0)
	batch := w.db.Batch()
	defer batch.Rollback()
	for _, tx := range txs {
		fp, err := w.putTx(batch, tx, height)
		if err != nil {
			return 0, err
		}
		if fp {
			fps++
		}
	}
	if err := batch.Commit(); err != nil {
		return 0, err
	}
	return fps, nil
}

// PutForkTxs persists the fork chain transactions into database with the
// fork block hash and can be queried by GetForkTxs(hash).
func (w *spvwallet) PutForkTxs(txs []util.Transaction, hash *common.Uint256) error {
	ftxs := make([]*util.Tx, 0, len(txs))
	for _, tx := range txs {
		ftxs = append(ftxs, util.NewTx(tx, 0))
	}
	return w.db.Txs().PutForkTxs(ftxs, hash)
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (w *spvwallet) HaveTx(txId *common.Uint256) (bool, error) {
	tx, err := w.db.Txs().Get(txId)
	return tx != nil, err
}

// GetTxs returns all transactions in main chain within the given height.
func (w *spvwallet) GetTxs(height uint32) ([]util.Transaction, error) {
	txs, err := w.db.Txs().GetAllFrom(height)
	if err != nil {
		return nil, err
	}

	utxs := make([]util.Transaction, 0, len(txs))
	for _, tx := range txs {
		wtx := newTransaction()
		if err := wtx.Deserialize(bytes.NewReader(tx.RawData)); err != nil {
			return nil, err
		}
		utxs = append(utxs, wtx)
	}
	return utxs, nil
}

// GetForkTxs returns all transactions within the fork block hash.
func (w *spvwallet) GetForkTxs(hash *common.Uint256) ([]util.Transaction, error) {
	ftxs, err := w.db.Txs().GetForkTxs(hash)
	if err != nil {
		return nil, err
	}

	txs := make([]util.Transaction, 0, len(ftxs))
	for _, ftx := range ftxs {
		tx := newTransaction()
		if err := tx.Deserialize(bytes.NewReader(ftx.RawData)); err != nil {
			return nil, err
		}
		txs = append(txs, tx)
	}
	return txs, nil
}

// DelTxs remove all transactions in main chain within the given height.
func (w *spvwallet) DelTxs(height uint32) error {
	batch := w.db.Batch()
	defer batch.Rollback()
	if err := batch.RollbackHeight(height); err != nil {
		return err
	}
	return batch.Commit()
}

// Clear delete all data in database.
func (w *spvwallet) Clear() error {
	return w.db.Clear()
}

// Close database.
func (w *spvwallet) Close() error {
	return w.db.Close()
}

func (w *spvwallet) GetFilter() *msg.TxFilterLoad {
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

	addrs := w.getAddrFilter().GetAddrs()

	elements := uint32(len(addrs) + len(outpoints))

	f := bloom.NewFilter(elements, 0, 0)
	for _, addr := range addrs {
		f.Add(addr.Bytes())
	}

	for _, op := range outpoints {
		f.Add(op.Bytes())
	}

	return f.ToTxFilterMsg(filter.FTBloom)
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

// Functions for RPC service.
func (w *spvwallet) notifyNewAddress(params http.Params) (interface{}, error) {
	addrStr, ok := params.String("addr")
	if !ok {
		return nil, ErrInvalidParameter
	}

	address, err := common.Uint168FromAddress(addrStr)
	if err != nil {
		return nil, err
	}

	waltlog.Debugf("receive notifyNewAddress %s", address)

	// Reload address filter to include new address
	w.loadAddrFilter()

	// Broadcast filterload message to connected peers
	w.UpdateFilter()

	return nil, nil
}

func (w *spvwallet) sendTransaction(params http.Params) (interface{}, error) {
	data, ok := params.String("data")
	if !ok {
		return nil, ErrInvalidParameter
	}

	txBytes, err := hex.DecodeString(data)
	if err != nil {
		return nil, ErrInvalidParameter
	}

	var tx = newTransaction()
	err = tx.Deserialize(bytes.NewReader(txBytes))
	if err != nil {
		return nil, fmt.Errorf("deserialize transaction failed %s", err)
	}

	return nil, w.SendTransaction(tx)
}

func NewWallet(dataDir string) (*spvwallet, error) {
	// Initialize headers db
	headers, err := headers.NewDatabase(dataDir)
	if err != nil {
		return nil, err
	}

	db, err := sqlite.NewDatabase(dataDir)
	if err != nil {
		return nil, err
	}

	w := spvwallet{db: db}
	chainStore := database.NewChainDB(headers, &w)

	var params *config.Params
	switch cfg.Network {
	case "testnet", "test", "t":
		params = config.DefaultParams.TestNet()
	case "regnet", "reg", "r":
		params = config.DefaultParams.RegNet()
	default:
		params = &config.DefaultParams
	}

	// Initialize spv service
	w.IService, err = sdk.NewService(&sdk.Config{
		ChainParams:    params,
		PermanentPeers: cfg.PermanentPeers,
		GenesisHeader:  sutil.NewHeader(&params.GenesisBlock.Header),
		ChainStore:     chainStore,
		NewTransaction: newTransaction,
		NewBlockHeader: sutil.NewEmptyHeader,
		GetTxFilter:    w.GetFilter,
		StateNotifier:  &w,
	})
	if err != nil {
		return nil, err
	}

	s := jsonrpc.NewServer(&jsonrpc.Config{
		Path:      "/spvwallet",
		ServePort: cfg.RPCPort,
	})
	s.RegisterAction("notifynewaddress", w.notifyNewAddress, "addr")
	s.RegisterAction("sendrawtransaction", w.sendTransaction, "data")
	go s.Start()

	return &w, nil
}

func newTransaction() util.Transaction {
	return sutil.NewTx(&types.Transaction{})
}
