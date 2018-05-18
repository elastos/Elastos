package spvwallet

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/store"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/core"
)

const MaxUnconfirmedTime = time.Minute * 1

func Init(clientId uint64, seeds []string) (*SPVWallet, error) {
	var err error
	wallet := new(SPVWallet)

	// Initialize headers db
	wallet.headerStore, err = db.NewHeadersDB()
	if err != nil {
		return nil, err
	}

	// Initialize wallet database
	wallet.dataStore, err = db.NewSQLiteDB()
	if err != nil {
		return nil, err
	}

	// Initialize P2P network client
	client, err := sdk.GetSPVClient(config.Values().Magic, clientId, seeds)
	if err != nil {
		return nil, err
	}

	// Initialize spv service
	wallet.SPVService, err = sdk.GetSPVService(client, wallet.headerStore, wallet)
	if err != nil {
		return nil, err
	}

	// Initialize RPC server
	server := rpc.InitServer()
	server.NotifyNewAddress = wallet.NotifyNewAddress
	server.SendTransaction = wallet.SPVService.SendTransaction
	wallet.rpcServer = server

	return wallet, nil
}

type DataListener interface {
	OnNewBlock(block *msg.MerkleBlock, txs []*core.Transaction)
	OnRollback(height uint32)
}

type SPVWallet struct {
	sync.Mutex
	sdk.SPVService
	rpcServer     *rpc.Server
	chainState    sdk.ChainState
	headerStore   *db.HeadersDB
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

func (wallet *SPVWallet) HeaderStore() store.HeaderStore {
	return wallet.headerStore
}

func (wallet *SPVWallet) DataStore() db.DataStore {
	return wallet.dataStore
}

func (wallet *SPVWallet) AddDataListener(listener DataListener) {
	wallet.dataListeners = append(wallet.dataListeners, listener)
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

func (wallet *SPVWallet) OnStateChange(state sdk.ChainState) {
	log.Debugf("On chain state change %s", state.String())
	wallet.chainState = state
}

// Commit a transaction return if this is a false positive and error
func (wallet *SPVWallet) CommitTx(tx *core.Transaction, height uint32) (bool, error) {
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
	err := wallet.dataStore.Txs().Put(db.NewTx(*tx, height))
	if err != nil {
		return false, err
	}

	return false, nil
}

func (wallet *SPVWallet) OnBlockCommitted(block *msg.MerkleBlock, txs []*core.Transaction) {
	wallet.dataStore.Chain().PutHeight(block.Header.(*core.Header).Height)
	for _, listener := range wallet.dataListeners {
		go listener.OnNewBlock(block, txs)
	}

	log.Debugf("On block committed chain state %s", wallet.chainState.String())
	// Check unconfirmed transaction timeout
	if wallet.chainState == sdk.WAITING {
		err := wallet.dataStore.RollbackTimeoutTxs(MaxUnconfirmedTime)
		if err != nil {
			log.Errorf("Rollback timeout transactions failed, error %s", err.Error())
		}
	}
}

// Rollback chain data on the given height
func (wallet *SPVWallet) OnRollback(height uint32) error {
	for _, listener := range wallet.dataListeners {
		go listener.OnRollback(height)
	}
	return wallet.dataStore.Rollback(height)
}

func ToUTXO(txId common.Uint256, height uint32, index int, value common.Fixed64, lockTime uint32) *db.UTXO {
	utxo := new(db.UTXO)
	utxo.Op = *core.NewOutPoint(txId, uint16(index))
	utxo.Value = value
	utxo.LockTime = lockTime
	utxo.AtHeight = height
	return utxo
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
