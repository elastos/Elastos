package _interface

import "github.com/elastos/Elastos.ELA.SPV/p2p"

/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// In this method you will set the peer parameters like clientId, port, services, relay etc.
	InitLocalPeer(func(*p2p.Peer))

	// Set the message handler
	SetMessageHandler(p2p.MessageHandler)

	// Start the P2P client
	Start()

	// Get the peer manager of this P2P client
	PeerManager() *p2p.PeerManager
}

func NewP2PClient(magic uint32, seeds []string) P2PClient {
	client := new(P2PClientImpl)
	client.magic = magic
	client.seeds = seeds
	return client
}
