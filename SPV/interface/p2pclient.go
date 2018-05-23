package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/net"
)

/*
P2P client is the interface to interactive with the peer to peer network implementation,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// In this method you will set the peer parameters like clientId, port, services, relay etc.
	InitLocalPeer(func(*net.Peer))

	// Set the message handler
	SetMessageHandler(handler net.MessageHandler)

	// Start the P2P client
	Start()

	// Get the peer manager of this P2P client
	PeerManager() *net.PeerManager
}

func NewP2PClient(magic uint32, seeds []string, maxOutbound, maxConnections int) P2PClient {
	return NewP2PClientImpl(magic, seeds, maxOutbound, maxConnections)
}
