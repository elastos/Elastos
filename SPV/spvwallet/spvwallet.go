package spvwallet

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/config"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/db"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/rpc"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/core"
)

const (
	MaxUnconfirmedTime = time.Minute * 30
	MaxTxIdCached      = 1000
	MaxConnections     = 12
)

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

	// Initialize txs cache
	wallet.txIds = NewTxIdCache(MaxTxIdCached)

	// Initialize P2P network client
	client, err := sdk.GetSPVClient(config.Values().Magic, clientId, seeds, MaxConnections, MaxConnections)
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
	rpcServer   *rpc.Server
	headerStore *db.HeadersDB
	dataStore   db.DataStore
	txIds       *TxIdCache
	filter      *sdk.AddrFilter
}

func (wallet *SPVWallet) Start() {
	wallet.SPVService.Start()
	wallet.rpcServer.Start()
}

func (wallet *SPVWallet) Stop() {
	wallet.SPVService.Stop()
	wallet.rpcServer.Close()
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

// Commit a transaction return if this is a false positive and error
func (wallet *SPVWallet) CommitTx(tx *core.Transaction, height uint32) (bool, error) {
	txId := tx.Hash()

	sh, ok := wallet.txIds.Get(txId)
	if ok && (sh > 0 || (sh == 0 && height == 0)) {
		return false, nil
	}

	// Do not check double spends when syncing
	if wallet.SPVService.ChainState() == sdk.WAITING {
		dubs, err := wallet.checkDoubleSpends(tx)
		if err != nil {
			return false, nil
		}
		if len(dubs) > 0 {
			if height == 0 {
				return false, nil
			} else {
				// Rollback any double spend transactions
				for _, dub := range dubs {
					if err := wallet.dataStore.RollbackTx(dub); err != nil {
						return false, nil
					}
				}
			}
		}
	}

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

	wallet.txIds.Add(txId, height)

	return false, nil
}

func (wallet *SPVWallet) OnBlockCommitted(block *msg.MerkleBlock, txs []*core.Transaction) {
	wallet.dataStore.Chain().PutHeight(block.Header.(*core.Header).Height)

	// Check unconfirmed transaction timeout
	if wallet.ChainState() == sdk.WAITING {
		// Get all unconfirmed transactions
		txs, err := wallet.dataStore.Txs().GetAllFrom(0)
		if err != nil {
			log.Debugf("Get unconfirmed transactions failed, error %s", err.Error())
			return
		}
		now := time.Now()
		for _, tx := range txs {
			if now.After(tx.Timestamp.Add(MaxUnconfirmedTime)) {
				err = wallet.dataStore.RollbackTx(&tx.TxId)
				if err != nil {
					log.Errorf("Rollback timeout transaction %s failed, error %s", tx.TxId.String(), err.Error())
				}
			}
		}
	}
}

// Rollback chain data on the given height
func (wallet *SPVWallet) OnRollback(height uint32) error {
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

// checkDoubleSpends takes a transaction and compares it with
// all transactions in the db.  It returns a slice of all txIds in the db
// which are double spent by the received tx.
func (wallet *SPVWallet) checkDoubleSpends(tx *core.Transaction) ([]*common.Uint256, error) {
	var dubs []*common.Uint256
	txId := tx.Hash()
	txs, err := wallet.dataStore.Txs().GetAll()
	if err != nil {
		return nil, err
	}
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
		for _, txIn := range tx.Inputs {
			for _, compIn := range compTx.Data.Inputs {
				if txIn.Previous.IsEqual(compIn.Previous) {
					// Found double spend
					dubs = append(dubs, &compTxId)
					break // back to txIn loop
				}
			}
		}
	}
	return dubs, nil
}
