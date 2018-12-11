package server

import (
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/peer"
)

// Ensure serverPeer implements the IPeer interface.
var _ IPeer = (*serverPeer)(nil)

// IPeer represents a peer for the server.
//
// The interface contract requires that all of these methods are safe for
// concurrent access.
type IPeer interface {
	// ToPeer returns the underlying peer instance.
	ToPeer() *peer.Peer

	// AddBanScore increases the persistent and decaying ban score fields by the
	// values passed as parameters. If the resulting score exceeds half of the ban
	// threshold, a warning is logged including the reason provided. Further, if
	// the score is above the ban threshold, the peer will be banned and
	// disconnected.
	AddBanScore(persistent, transient uint32, reason string)

	// BanScore returns the current integer value that represents how close
	// the peer is to being banned.
	BanScore() uint32
}

// Ensure server implements the IServer interface.
var _ IServer = (*server)(nil)

// IServer represents a server.
//
// The interface contract requires that all of these methods are safe for
// concurrent access.
type IServer interface {
	// Start begins accepting connections from peers.
	Start()

	// Stop gracefully shuts down the server by stopping and disconnecting all
	// peers and the main listener.
	Stop() error

	// ScheduleShutdown schedules a server shutdown after the specified duration.
	// It also dynamically adjusts how often to warn the server is going down based
	// on remaining duration.
	ScheduleShutdown(duration time.Duration)

	// Connect adds the provided address as a new outbound peer.  The
	// permanent flag indicates whether or not to make the peer persistent
	// and reconnect if the connection is lost.  Attempting to connect to an
	// already existing peer will return an error.
	Connect(addr string, permanent bool) error

	// RemoveByID removes the peer associated with the provided id from the
	// list of persistent peers.  Attempting to remove an id that does not
	// exist will return an error.
	RemoveByID(id uint64) error

	// RemoveByAddr removes the peer associated with the provided address
	// from the list of persistent peers.  Attempting to remove an address
	// that does not exist will return an error.
	RemoveByAddr(addr string) error

	// DisconnectByID disconnects the peer associated with the provided id.
	// This applies to both inbound and outbound peers.  Attempting to
	// remove an id that does not exist will return an error.
	DisconnectByID(id uint64) error

	// DisconnectByAddr disconnects the peer associated with the provided
	// address.  This applies to both inbound and outbound peers.
	// Attempting to remove an address that does not exist will return an
	// error.
	DisconnectByAddr(addr string) error

	// ConnectedCount returns the number of currently connected peers.
	ConnectedCount() int32

	// ConnectedPeers returns an array consisting of all connected peers.
	ConnectedPeers() []IPeer

	// PersistentPeers returns an array consisting of all the persistent
	// peers.
	PersistentPeers() []IPeer

	// BroadcastMessage sends the provided message to all currently
	// connected peers.
	BroadcastMessage(msg p2p.Message, exclPeers ...*serverPeer)
}

// NewServer return a server instance that implement the IServer interface.
func NewServer(cfg *Config) (IServer, error) {
	return newServer(cfg)
}
