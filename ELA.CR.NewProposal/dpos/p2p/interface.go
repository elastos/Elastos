package p2p

import (
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// PeerAddr represent a connect peer's ID and it's IP address
type PeerAddr struct {
	// PID is the peer's public key id.
	PID common.Uint256

	// Addr is the peer's IP address.  It can be host:port format,
	// or host only and use the DefaultPort passed by server config.
	Addr string
}

// Peer represent the connected peer.
type Peer interface {
	// PID returns the peer's public key id.
	PID() common.Uint256

	// ToPeer returns the real peer instance.
	ToPeer() *peer.Peer
}

// StateNotifier notifies the server peer state changes.
type StateNotifier interface {
	// OnStateChange will be invoked when peers connected or disconnected.
	//
	// Notify: do not modify the invoked state value.  It's read only.
	OnStateChange(state *peerState)
}

// Server provides a server handling connections to and from peers.
type Server interface {
	// Start begins accepting connections from peers.
	Start()

	// Stop gracefully shuts down the server by stopping and disconnecting all
	// peers and the main listener.
	Stop() error

	// ConnectPeers let server connect the peers in the given addrList, and
	// disconnect peers that not in the addrList.
	ConnectPeers(addrList []PeerAddr)

	// SendMessageToPeer send a message to the peer with the given id, error
	// will be returned if there is no matches, or fail to send the message.
	SendMessageToPeer(id common.Uint256, msg p2p.Message) error

	// BroadcastMessage sends msg to all peers currently connected to the server
	// except those in the passed peers to exclude.
	BroadcastMessage(msg p2p.Message, exclPeers ...common.Uint256)

	// ConnectedPeers returns an array consisting of all connected peers.
	ConnectedPeers() []Peer
}
