package _interface

import "github.com/elastos/Elastos.ELA.SPV/p2p"

/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// In this method you will set the peer parameters like clientId, port, services, relay etc.
	InitLocalPeer(func(*p2p.Peer))

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

func NewP2PClient(magic uint32, seeds []string) P2PClient {
	client := new(P2PClientImpl)
	client.magic = magic
	client.seeds = seeds
	return client
}
