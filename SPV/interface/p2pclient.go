package _interface

import "SPVWallet/p2p"

/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Start the P2P client
	Start()

	// Handle the version message witch includes information of a handshake peer
	HandleVersion(callback func(v *p2p.Version) error)

	// Handle a new peer connect
	PeerConnected(callback func(peer *p2p.Peer))

	// Make a message instance with the given cmd
	MakeMessage(callback func(cmd string) (p2p.Message, error))

	// Handle a message from a connected peer
	HandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error)

	// Get the peer manager of this P2P client
	PeerManager() *p2p.PeerManager
}

func NewP2PClient(clientId uint64, magic uint32, port uint16, seeds []string) P2PClient {
	client := new(P2PClientImpl)
	client.id = clientId
	client.magic = magic
	client.port = port
	client.seeds = seeds
	return client
}
