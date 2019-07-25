package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

/*
IService is an implementation for SPV features.
*/
type IService interface {
	// Start SPV service
	Start()

	// Stop SPV service
	Stop()

	// IsCurrent returns whether or not the SPV service believes it is synced with
	// the connected peers.
	IsCurrent() bool

	// UpdateFilter is a trigger to make SPV service refresh the current
	// transaction filer(in our implementation the bloom filter) and broadcast the
	// new filter to connected peers.  This will invoke the GetFilterData() method
	// in Config.
	UpdateFilter()

	// SendTransaction broadcast a transaction message to the peer to peer network.
	SendTransaction(util.Transaction) error
}

// StateNotifier exposes methods to notify status changes of transactions and blocks.
type StateNotifier interface {
	// TransactionAnnounce will be invoked when received a new announced transaction.
	TransactionAnnounce(tx util.Transaction)

	// TransactionAccepted will be invoked after a transaction sent by
	// SendTransaction() method has been accepted.  Notice: this method needs at
	// lest two connected peers to work.
	TransactionAccepted(tx util.Transaction)

	// TransactionRejected will be invoked if a transaction sent by SendTransaction()
	// method has been rejected.
	TransactionRejected(tx util.Transaction)

	// TransactionConfirmed will be invoked after a transaction sent by
	// SendTransaction() method has been packed into a block.
	TransactionConfirmed(tx *util.Tx)

	// BlockCommitted will be invoked when a block and transactions within it are
	// successfully committed into database.
	BlockCommitted(block *util.Block)
}

// Config is the configuration settings to the SPV service.
type Config struct {
	// DataDir is the data path to store peer addresses etc.
	DataDir string

	// ChainParams indicates the network parameters for the SPV service.
	ChainParams *config.Params

	// PermanentPeers are the peers need to be connected permanently.
	PermanentPeers []string

	// CandidateFlags defines flags needed for a sync candidate.
	CandidateFlags []uint64

	// GenesisHeader is the
	GenesisHeader util.BlockHeader

	// The database to store all block headers
	ChainStore database.ChainStore

	// NewTransaction create a new transaction instance.
	NewTransaction func() util.Transaction

	// NewBlockHeader create a new block header instance.
	NewBlockHeader func() util.BlockHeader

	// GetTxFilter() returns a transaction filter like a bloom filter or others.
	GetTxFilter func() *msg.TxFilterLoad

	// StateNotifier is an optional config, if you don't want to receive state changes of transactions
	// or blocks, just keep it blank.
	StateNotifier StateNotifier
}

/*
NewService returns a new SPV service instance.
there are two implementations you need to do, DataStore and GetBloomFilter() method.
DataStore is an interface including all methods you need to implement placed in db/datastore.go.
Also an sample APP spvwallet is contain in this project placed in spvwallet folder.
*/
func NewService(config *Config) (IService, error) {
	return newService(config)
}
