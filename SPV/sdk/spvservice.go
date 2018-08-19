package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/store"

	"github.com/elastos/Elastos.ELA.SPV/net"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	ela "github.com/elastos/Elastos.ELA/core"
)

/*
SPV service is a high level implementation with all SPV logic implemented.
SPV service is extend from SPV client and implement Blockchain and block synchronize on it.
With SPV service, you just need to implement your own HeaderStore and SPVServiceConfig, and let other stuff go.
*/
type SPVService interface {
	// Start SPV service
	Start()

	// Stop SPV service
	Stop()

	// ChainState returns the current state of the blockchain, to indicate that the blockchain
	// is in syncing mode or waiting mode.
	ChainState() ChainState

	// ReloadFilters is a trigger to make SPV service refresh the current
	// transaction filer(in our implementation the bloom filter) in SPV service.
	// This will call onto the GetAddresses() and GetOutpoints() method in SPVServiceConfig.
	ReloadFilter()

	// SendTransaction broadcast a transaction message to the peer to peer network.
	SendTransaction(ela.Transaction) (*common.Uint256, error)
}

type SPVServiceConfig struct {
	// The server peer access into blockchain peer to peer network
	Server *net.ServerPeer

	// Foundation address of the current access blockhain network
	Foundation string

	// The database to store all block headers
	HeaderStore store.HeaderStore

	// GetFilterData() returns two arguments.
	// First arguments are all addresses stored in your data store.
	// Second arguments are all balance references to those addresses stored in your data store,
	// including UTXO(Unspent Transaction Output)s and STXO(Spent Transaction Output)s.
	// Outpoint is a data structure include a transaction ID and output index. It indicates the
	// reference of an transaction output. If an address ever received an transaction output,
	// there will be the outpoint reference to it. Any time you want to spend the balance of an
	// address, you must provide the reference of the balance which is an outpoint in the transaction input.
	GetFilterData func() ([]*common.Uint168, []*ela.OutPoint)

	// When interested transactions received, this method will call back them.
	// The height is the block height where this transaction has been packed.
	// Returns if the transaction is a match, for there will be transactions that
	// are not interested go through this method. If a transaction is not a match
	// return false as a false positive mark. If anything goes wrong, return error.
	// Notice: this method will be callback when commit block
	CommitTx func(tx *ela.Transaction, height uint32) (bool, error)

	// This method will be callback after a block and transactions with it are
	// successfully committed into database.
	OnBlockCommitted func(*msg.MerkleBlock, []*ela.Transaction)

	// When the blockchain meet a reorganization, data should be rollback to the fork point.
	// The Rollback method will callback the current rollback height, for example OnChainRollback(100)
	// means data on height 100 has been deleted, current chain height will be 99. You should rollback
	// stored data including UTXOs STXOs Txs etc. according to the given height.
	// If anything goes wrong, return an error.
	OnRollback func(height uint32) error
}

/*
Get a SPV service instance.
there are two implementations you need to do, DataStore and GetBloomFilter() method.
DataStore is an interface including all methods you need to implement placed in db/datastore.go.
Also an sample APP spvwallet is contain in this project placed in spvwallet folder.
*/
func GetSPVService(config SPVServiceConfig) (SPVService, error) {
	return NewSPVServiceImpl(config)
}
