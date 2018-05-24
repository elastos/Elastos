package sdk

import (
	"github.com/elastos/Elastos.ELA.SPV/net"
)

/*
P2P client is the interface to interactive with the peer to peer network,
use this to join the peer to peer network and make communication with other peers.
*/
type P2PClient interface {
	// Start the P2P client
	Start()

	// Get the peer manager of this P2P client
	PeerManager() *net.PeerManager
}

// To get a P2P client, you need to set a magic number and a client ID to identify this peer in the peer to peer network.
// Magic number is the peer to peer network id for the peers in the same network to identify each other,
// and client id is the unique id to identify the current peer in this peer to peer network.
// seeds is a list which is the other peers IP:Port addresses.
// port is the port number for this client listening inbound connections.
func GetP2PClient(magic uint32, clientId uint64, seeds []string, port uint16, minOutbound, maxConnections int) (P2PClient, error) {
	return NewP2PClientImpl(magic, clientId, seeds, port, minOutbound, maxConnections)
}
