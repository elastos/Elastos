package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/db"
	"github.com/elastos/Elastos.ELA.SPV/bloom"
	"github.com/elastos/Elastos.ELA.SPV/p2p"
)

/*
SPV service is a high level implementation with all SPV logic implemented.
SPV service is extend from SPV client and implement Blockchain and block synchronize on it.
With SPV service, you just need to implement your own DataStore and GetBloomFilter() method, and let other stuff go.
*/
type SPVService interface {
	// Start SPV service
	Start()

	// Stop SPV service
	Stop()

	// Get the Blockchain instance.
	// Blockchain will handle block and transaction commits,
	// verify and store the block and transactions.
	// If you want to add extra logic when new block or transaction comes,
	// use Blockchain.AddStateListener() to register chain state callbacks
	Blockchain() *Blockchain

	// Broadcast a message to the peer to peer network.
	BroadCastMessage(message p2p.Message)
}

/*
Get a SPV service instance.
there are two implementations you need to do, DataStore and GetBloomFilter() method.
DataStore is an interface including all methods you need to implement placed in db/datastore.go.
Also an sample APP spvwallet is contain in this project placed in spvwallet folder.
*/
func GetSPVService(client SPVClient, database db.DataStore, getBloomFilter func() *bloom.Filter) (SPVService, error) {
	return NewSPVServiceImpl(client, database, getBloomFilter)
}
