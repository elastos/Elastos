package sdk

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/p2p"
	"github.com/elastos/Elastos.ELA.SPV/msg"
)

/*
SPV client will help you create and receive SPV messages,
and you will implement your own message handler to extend the SDK.
A SPV wallet implementation is in the spvwallet package,
you can see how to extend the SDK and create your own apps.
*/
type SPVClient interface {
	// Set the message handler to extend the client
	SetMessageHandler(SPVMessageHandler)

	// Start the client
	Start()

	// Get peer manager, which is the main program of the peer to peer network
	PeerManager() *p2p.PeerManager

	// Create a blocks request message using block locator and stop hash
	NewBlocksReq(locator []*Uint256, hashStop Uint256) *msg.BlocksReq

	// Create a data request message, invType is TRANSACTION or BLOCK according to the SPV protocol
	// the inv type constant is in the protocol file
	NewDataReq(invType uint8, hash Uint256) *msg.DataReq
}

// The message handler to extend the SDK
type SPVMessageHandler interface {
	// When a peer is connected and established
	// this method will callback to pass the connected peer
	OnPeerEstablish(*p2p.Peer)

	// After send a blocks request message, this inventory message
	// will return with a bunch of block hashes, then you can use them
	// to request all the blocks by send data requests.
	OnInventory(*p2p.Peer, *msg.Inventory) error

	// After sent a data request with invType BLOCK, a merkleblock message will return through this method.
	// To make this work, you must register a filterload message to the connected peer first,
	// then this client will be known as a SPV client. To create a bloom filter and get the
	// filterload message, you will use the method in SDK bloom sdk.NewBloomFilter()
	// merkleblock includes a block header, transaction hashes in merkle proof format.
	// Which transaction hashes will be in the merkleblock is depends on the addresses and outpoints
	// you've added into the bloom filter before you send a filterload message with this bloom filter.
	// You will use these transaction hashes to request transactions by sending data request message
	// with invType TRANSACTION
	OnMerkleBlock(*p2p.Peer, *bloom.MerkleBlock) error

	// After sent a data request with invType TRANSACTION, a txn message will return through this method.
	// these transactions are matched to the bloom filter you have sent with the filterload message.
	OnTxn(*p2p.Peer, *msg.Txn) error

	// If the BLOCK or TRANSACTION requested by the data request message can not be found,
	// notfound message with requested data hash will return through this method.
	OnNotFound(*p2p.Peer, *msg.NotFound) error
}

/*
Get the SPV client by specify the netType, passing the clientId and seeds arguments.
netType are TypeMainNet and TypeTestNet two options, clientId is the unique id to identify
this client in the peer to peer network. seeds is a list of other peers IP:[Port] addresses,
port is not necessary for it will be overwrite to SPVServerPort according to the SPV protocol
*/
func GetSPVClient(netType string, clientId uint64, seeds []string) (SPVClient, error) {
	var magic uint32
	switch netType {
	case TypeMainNet:
		magic = MainNetMagic
	case TypeTestNet:
		magic = TestNetMagic
	default:
		return nil, errors.New("Unknown net type ")
	}
	return NewSPVClientImpl(magic, clientId, seeds)
}
