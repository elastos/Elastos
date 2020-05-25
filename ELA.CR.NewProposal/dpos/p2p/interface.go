// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package p2p

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p"
)

// ConnState indicates the peer connection state.
type ConnState uint8

const (
	// CSNoneConnection indicates the peer has no connection.
	CSNoneConnection ConnState = iota

	// CSOutboundOnly indicates the peer has outbound connection only.
	CSOutboundOnly

	// CSInboundOnly indicates the peer has inbound connection only.
	CSInboundOnly

	// CS2WayConnection indicates the peer have both inbound and outbound
	// connections.
	CS2WayConnection
)

// Names of ConnState for pretty printing.
var csStrings = []string{
	"NoneConnection",
	"OutboundOnly",
	"InboundOnly",
	"2WayConnection",
}

// String returns the ConnState in human-readable form.
func (cs ConnState) String() string {
	if int(cs) <= len(csStrings)-1 {
		return csStrings[cs]
	}
	return fmt.Sprintf("ConnState%d", cs)
}

// PeerAddr represent a DPOS peer's ID and it's network address
type PeerAddr struct {
	// PID is the peer's public key id.
	PID peer.PID

	// Addr is the peer's network address in host:port format.
	Addr string
}

// Peer represent the connected peer.
type Peer interface {
	// PID returns the peer's public key id.
	PID() peer.PID

	// ToPeer returns the real peer instance.
	ToPeer() *peer.Peer
}

// PeerInfo represent the peer info of the connect peers.
type PeerInfo struct {
	// PID is the peer's public key id.
	PID peer.PID

	// Addr is the peer's IP address.  It can be host:port format,
	// or host only and use the DefaultPort passed by server config.
	Addr string

	// State is the peer's connection state.
	State ConnState
}

// StateNotifier notifies the server peer state changes.
type StateNotifier interface {
	// OnConnectPeers will be invoked when server received a connect peers
	// message.
	//
	// Notify: do not modify the invoked list.  It's read only.
	OnConnectPeers(peers []peer.PID)

	// OnNewPeer will be invoked when a new peer negotiated.
	OnNewPeer(pid peer.PID)

	// OnDonePeer will be invoked when a peer disconnected.
	OnDonePeer(pid peer.PID)
}

// Server provides a server handling connections to and from peers.
type Server interface {
	// Start begins accepting connections from peers.
	Start()

	// Stop gracefully shuts down the server by stopping and disconnecting all
	// peers and the main listener.
	Stop() error

	// AddAddr adds an arbiter address into AddrManager.
	AddAddr(pid peer.PID, addr string)

	// ConnectPeers let server connect the peers in the given list, and
	// disconnect peers that not in the list.
	ConnectPeers(peers []peer.PID)

	// SendMessageToPeer send a message to the peer with the given id, error
	// will be returned if there is no matches, or fail to send the message.
	SendMessageToPeer(pid peer.PID, msg p2p.Message) error

	// BroadcastMessage sends msg to all peers currently connected to the server
	// except those in the passed peers to exclude.
	BroadcastMessage(msg p2p.Message, exclPeers ...peer.PID)

	// ConnectedPeers returns an array consisting of all connected peers.
	ConnectedPeers() []Peer

	// DumpPeersInfo returns a list of connect peers information.  This is a
	// high cost method, should not be called frequently.
	DumpPeersInfo() []*PeerInfo
}
