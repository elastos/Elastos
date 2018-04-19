package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

/*
P2P client is the interface to interactive with the peer to peer network,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Set the peer to peer message handler
	SetMessageHandler(handler P2PMessageHandler)

	// Start the P2P client
	Start()

	// Get the peer manager of this P2P client
	PeerManager() *net.PeerManager
}

// Handle the message creation, allocation etc.
type P2PMessageHandler interface {
	// Create a message instance by the given cmd parameter
	MakeMessage(cmd string) (p2p.Message, error)

	// VerAck message received from a connected peer
	// which means the connected peer is established
	OnPeerEstablish(*net.Peer)

	// Handle messages received from the connected peer
	HandleMessage(*net.Peer, p2p.Message) error
}

// To get a P2P client, you need to set a magic number and a client ID to identify this peer in the peer to peer network.
// Magic number is the peer to peer network id for the peers in the same network to identify each other,
// and client id is the unique id to identify the current peer in this peer to peer network.
// seeds is a list which is the other peers IP:[Port] addresses,
// port is not necessary for it will be overwrite to SPVServerPort according to the SPV protocol
func GetP2PClient(magic uint32, clientId uint64, seeds []string) (P2PClient, error) {
	return NewP2PClientImpl(magic, clientId, seeds)
}
