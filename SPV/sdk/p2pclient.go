package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/p2p"
)

/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Start the P2P client
	Start()

	// Handle a new peer connect
	OnPeerConnected(callback func(peer *p2p.Peer))

	// Make a message instance with the given cmd
	OnMakeMessage(callback func(cmd string) (p2p.Message, error))

	// Handle a message from a connected peer
	OnHandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error)

	// Get the peer manager of this P2P client
	PeerManager() *p2p.PeerManager
}

// To get a P2P client, you need to set a magic number and a client ID to identify this peer in the peer to peer network.
// Magic number is the peer to peer network id for the peers in the same network to identify each other,
// and client id is the unique id to identify the current peer in this peer to peer network.
// seeds is a list witch is the other peers IP:[Port] addresses,
// port is not necessary for it will be overwrite to SPVServerPort according to the SPV protocol
func GetP2PClient(magic uint32, clientId uint64, seeds []string) (P2PClient, error) {
	return NewP2PClientImpl(magic, clientId, seeds)
}
