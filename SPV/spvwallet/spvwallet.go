package spvwallet

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	. "github.com/elastos/Elastos.ELA.SPV/db"
	"github.com/elastos/Elastos.ELA.SPV/msg"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
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
	wallet.SPVService, err = sdk.GetSPVService(client, wallet, wallet.getBloomFilter)
	if err != nil {
		return nil, err
	}

	// Initialize RPC server
	wallet.rpcServer = rpc.InitServer(wallet)

	return wallet, nil
}

type SPVWallet struct {
	sync.Mutex
	sdk.SPVService
	rpcServer *rpc.Server
	headers   db.Headers
	dataStore db.DataStore
	filter    *sdk.AddrFilter
}

func (wallet *SPVWallet) Start() {
	wallet.SPVService.Start()
	wallet.rpcServer.Start()
}

func (wallet *SPVWallet) Stop() {
	wallet.SPVService.Stop()
	wallet.rpcServer.Close()
}

func (wallet *SPVWallet) Headers() db.Headers {
	return wallet.headers
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
func (wallet *SPVWallet) GetHeader(hash common.Uint256) (*StoreHeader, error) {
	return wallet.headers.GetHeader(hash)
}

// Get the header on chain tip
func (wallet *SPVWallet) GetChainTip() (*StoreHeader, error) {
	return wallet.headers.GetTip()
}

// Save chain height to database
func (wallet *SPVWallet) PutChainHeight(height uint32) {
	wallet.dataStore.Info().SaveChainHeight(height)
}

// Get chain height from database
func (wallet *SPVWallet) GetChainHeight() uint32 {
	return wallet.dataStore.Info().ChainHeight()
}

// Commit a transaction return if this is a false positive and error
func (wallet *SPVWallet) CommitTx(storeTx *StoreTx) (bool, error) {
	hits := 0
	// Save UTXOs
	for index, output := range storeTx.Data.Outputs {
		// Filter address
		if wallet.getAddrFilter().ContainAddr(output.ProgramHash) {
			var lockTime uint32
			if storeTx.Data.TxType == tx.CoinBase {
				lockTime = storeTx.Height + 100
			}
			utxo := ToUTXO(storeTx.TxId, storeTx.Height, index, output.Value, lockTime)
			err := wallet.dataStore.UTXOs().Put(&output.ProgramHash, utxo)
			if err != nil {
				return false, err
			}
			hits++
		}
	}

	// Put spent UTXOs to STXOs
	for _, input := range storeTx.Data.Inputs {
		// Create output
		outpoint := tx.NewOutPoint(input.ReferTxID, input.ReferTxOutputIndex)
		// Try to move UTXO to STXO, if a UTXO in database was spent, it will be moved to STXO
		err := wallet.dataStore.STXOs().FromUTXO(outpoint, &storeTx.TxId, storeTx.Height)
		if err == nil {
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction
	err := wallet.dataStore.Txs().Put(storeTx)
	if err != nil {
		return false, err
	}

	return false, nil
}

// Rollback chain data on the given height
func (wallet *SPVWallet) Rollback(height uint32) error {
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
	utxo.Op = *tx.NewOutPoint(txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	return utxo
}

func (wallet *SPVWallet) NotifyNewAddress(hash []byte) error {
	// Reload address filter to include new address
	wallet.loadAddrFilter()
	// Broadcast filterload message to connected peers
	wallet.BroadCastMessage(wallet.getBloomFilter().GetFilterLoadMsg())
	return nil
}

func (wallet *SPVWallet) SendTransaction(tx tx.Transaction) error {
	// Broadcast transaction to connected peers
	wallet.BroadCastMessage(wallet.newTxnMsg(tx))
	return nil
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

func (wallet *SPVWallet) getBloomFilter() *bloom.Filter {
	wallet.Lock()
	defer wallet.Unlock()

	addrs := wallet.getAddrFilter().GetAddrs()
	utxos, _ := wallet.dataStore.UTXOs().GetAll()
	stxos, _ := wallet.dataStore.STXOs().GetAll()

	elements := uint32(len(addrs) + len(utxos) + len(stxos))
	filter := sdk.NewBloomFilter(elements)

	for _, addr := range addrs {
		filter.Add(addr.ToArray())
	}

	for _, utxo := range utxos {
		filter.AddOutPoint(&utxo.Op)
	}

	for _, stxo := range stxos {
		filter.AddOutPoint(&stxo.Op)
	}

	return filter
}

func (wallet *SPVWallet) newTxnMsg(tx tx.Transaction) *msg.Txn {
	return &msg.Txn{Transaction: tx}
}
