package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/core"
	"github.com/elastos/Elastos.ELA.SPV/store"

	ela "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

/*
SPV service is a high level implementation with all SPV logic implemented.
SPV service is extend from SPV client and implement Blockchain and block synchronize on it.
With SPV service, you just need to implement your own HeaderStore and SPVHandler, and let other stuff go.
*/
type SPVService interface {
	// Start SPV service
	Start()

	// Stop SPV service
	Stop()

	// ReloadFilters is a trigger to make SPV service refresh the current
	// transaction filer(in our implementation the bloom filter) in SPV service.
	// This will call onto the GetAddresses() and GetOutpoints() method in SPVHandler.
	ReloadFilter()

	// SendTransaction broadcast a transaction message to the peer to peer network.
	SendTransaction(ela.Transaction)
}

type SPVHandler interface {
	// SPV handler is extend from blockchain state listener
	core.StateListener

	// GetData returns two arguments.
	// First arguments are all addresses stored in your data store.
	// Second arguments are all balance references to those addresses stored in your data store,
	// including UTXO(Unspent Transaction Output)s and STXO(Spent Transaction Output)s.
	// Outpoint is a data structure include a transaction ID and output index. It indicates the
	// reference of an transaction output. If an address ever received an transaction output,
	// there will be the outpoint reference to it. Any time you want to spend the balance of an
	// address, you must provide the reference of the balance which is an outpoint in the transaction input.
	GetData() ([]*common.Uint168, []*ela.OutPoint)
}

/*
Get a SPV service instance.
there are two implementations you need to do, DataStore and GetBloomFilter() method.
DataStore is an interface including all methods you need to implement placed in db/datastore.go.
Also an sample APP spvwallet is contain in this project placed in spvwallet folder.
*/
func GetSPVService(client SPVClient, headerStore store.HeaderStore, handler SPVHandler) (SPVService, error) {
	return NewSPVServiceImpl(client, headerStore, handler)
}
