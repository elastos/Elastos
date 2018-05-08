package spvwallet

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/store"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"

	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

func Init(clientId uint64, seeds []string) (*SPVWallet, error) {
	var err error
	wallet := new(SPVWallet)

	// Initialize headers db
	wallet.headers, err = db.NewHeadersDB()
	if err != nil {
		return nil, err
	}

	// Initialize wallet database
	wallet.dataStore, err = db.NewSQLiteDB()
	if err != nil {
		return nil, err
	}

	// Initialize P2P network client
	client, err := sdk.GetSPVClient(sdk.TypeMainNet, clientId, seeds)
	if err != nil {
		return nil, err
	}

	// Initialize spv service
	wallet.SPVService, err = sdk.GetSPVService(client, wallet, wallet)
	if err != nil {
		return nil, err
	}

	// Initialize RPC server
	wallet.rpcServer = rpc.InitServer(wallet)

	return wallet, nil
}

type DataListener interface {
	OnNewBlock(block bloom.MerkleBlock, txs []core.Transaction)
	OnRollback(height uint32)
}

type SPVWallet struct {
	sync.Mutex
	sdk.SPVService
	rpcServer     *rpc.Server
	headers       db.Headers
	dataStore     db.DataStore
	filter        *sdk.AddrFilter
	dataListeners []DataListener
}

func (wallet *SPVWallet) Start() {
	wallet.SPVService.Start()
	wallet.rpcServer.Start()
}

func (wallet *SPVWallet) Stop() {
	wallet.SPVService.Stop()
	wallet.rpcServer.Close()
}

func (wallet *SPVWallet) HeaderStore() HeaderStore {
	return wallet
}

func (wallet *SPVWallet) DataStore() db.DataStore {
	return wallet.dataStore
}

// Save a header to database
func (wallet *SPVWallet) PutHeader(header *StoreHeader, newTip bool) error {
	return wallet.headers.Put(header, newTip)
}

// Get previous block of the given header
func (wallet *SPVWallet) GetPrevious(header *StoreHeader) (*StoreHeader, error) {
	return wallet.headers.GetPrevious(header)
}

// Get full header with it's hash
func (wallet *SPVWallet) GetHeader(hash *common.Uint256) (*StoreHeader, error) {
	return wallet.headers.GetHeader(hash)
}

// Get the header on chain tip
func (wallet *SPVWallet) GetBestHeader() (*StoreHeader, error) {
	return wallet.headers.GetTip()
}

func (wallet *SPVWallet) AddDataListener(listener DataListener) {
	wallet.dataListeners = append(wallet.dataListeners, listener)
}

// Commit a transaction return if this is a false positive and error
func (wallet *SPVWallet) OnCommitTx(tx core.Transaction, height uint32) (bool, error) {
	txId := tx.Hash()

	hits := 0
	// Save UTXOs
	for index, output := range tx.Outputs {
		// Filter address
		if wallet.getAddrFilter().ContainAddr(output.ProgramHash) {
			var lockTime uint32
			if tx.TxType == core.CoinBase {
				lockTime = height + 100
			}
			utxo := ToUTXO(txId, height, index, output.Value, lockTime)
			err := wallet.dataStore.UTXOs().Put(&output.ProgramHash, utxo)
			if err != nil {
				return false, err
			}
			hits++
		}
	}

	// Put spent UTXOs to STXOs
	for _, input := range tx.Inputs {
		// Try to move UTXO to STXO, if a UTXO in database was spent, it will be moved to STXO
		err := wallet.dataStore.STXOs().FromUTXO(&input.Previous, &txId, height)
		if err == nil {
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction
	err := wallet.dataStore.Txs().Put(db.NewTx(tx, height))
	if err != nil {
		return false, err
	}

	return false, nil
}

func (wallet *SPVWallet) OnBlockCommitted(block bloom.MerkleBlock, txs []core.Transaction) {
	wallet.dataStore.Chain().PutHeight(block.Height)
	for _, listener := range wallet.dataListeners {
		go listener.OnNewBlock(block, txs)
	}
}

// Rollback chain data on the given height
func (wallet *SPVWallet) OnRollback(height uint32) error {
	for _, listener := range wallet.dataListeners {
		go listener.OnRollback(height)
	}
	return wallet.dataStore.Rollback(height)
}

// Reset database, clear all data
func (wallet *SPVWallet) Reset() error {
	err := wallet.headers.Reset()
	if err != nil {
		return err
	}
	err = wallet.dataStore.Reset()
	if err != nil {
		return err
	}
	return nil
}

// Close the database
func (wallet *SPVWallet) Close() {
	wallet.headers.Close()
	wallet.dataStore.Close()
}

func ToUTXO(txId common.Uint256, height uint32, index int, value common.Fixed64, lockTime uint32) *db.UTXO {
	utxo := new(db.UTXO)
	utxo.Op = *core.NewOutPoint(txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	return utxo
}

func (wallet *SPVWallet) GetData() ([]*common.Uint168, []*core.OutPoint) {
	utxos, _ := wallet.dataStore.UTXOs().GetAll()
	stxos, _ := wallet.dataStore.STXOs().GetAll()

	outpoints := make([]*core.OutPoint, 0, len(utxos)+len(stxos))
	for _, utxo := range utxos {
		outpoints = append(outpoints, &utxo.Op)
	}
	for _, stxo := range stxos {
		outpoints = append(outpoints, &stxo.Op)
	}

	return wallet.getAddrFilter().GetAddrs(), outpoints
}

func (wallet *SPVWallet) NotifyNewAddress(hash []byte) {
	// Reload address filter to include new address
	wallet.loadAddrFilter()
	// Broadcast filterload message to connected peers
	wallet.ReloadFilter()
}

func (wallet *SPVWallet) getAddrFilter() *sdk.AddrFilter {
	if wallet.filter == nil {
		wallet.loadAddrFilter()
	}
	return wallet.filter
}

func (wallet *SPVWallet) loadAddrFilter() *sdk.AddrFilter {
	addrs, _ := wallet.dataStore.Addrs().GetAll()
	wallet.filter = sdk.NewAddrFilter(nil)
	for _, addr := range addrs {
		wallet.filter.AddAddr(addr.Hash())
	}
	return wallet.filter
}
