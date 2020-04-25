// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package p2p

import (
	"net"
	"time"

	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/dpos/p2p/peer"
	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// defaultConnectTimeout is the default duration we timeout a dial to peer.
	defaultConnectTimeout = time.Second * 30

	// defaultPingInterval is the default interval of time to wait in between
	// sending ping messages.
	defaultPingInterval = time.Second * 10
)

// Config is a descriptor which specifies the server instance configuration.
type Config struct {
	// DataDir is the data path to store peer addresses etc.
	DataDir string

	// PID is the public key id of this server.
	PID peer.PID

	// EnableHub indicates whether or not to enable the hub service.
	EnableHub bool

	// Localhost represents the local host IP or name of this peer.
	Localhost string

	// MagicNumber is the peer-to-peer network ID to connect to.
	MagicNumber uint32

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// TimeSource defines the median time source to use for things such as
	// view changing.
	TimeSource dtime.MedianTimeSource

	// ConnectTimeout is the duration before we timeout a dial to peer.
	ConnectTimeout time.Duration

	// PingInterval is the interval of time to wait in between sending ping
	// messages.
	PingInterval time.Duration

	// Signature will be invoked when creating a signature of the data content.
	Sign func(data []byte) (signature []byte)

	// PingNonce will be invoked before send a ping message to the connect peer
	// with the given PID, to get the nonce value within the ping message.
	PingNonce func(pid peer.PID) uint64

	// PongNonce will be invoked before send a pong message to the connect peer
	// with the given PID, to get the nonce value within the pong message.
	PongNonce func(pid peer.PID) uint64

	// MakeEmptyMessage will be invoked to creates a message of the appropriate
	// concrete type based on the command.
	MakeEmptyMessage func(command string) (p2p.Message, error)

	// HandleMessage will be invoked to handle the received message from
	// connected peers.  The peer's public key id will be pass together with
	// the received message.
	HandleMessage func(pid peer.PID, msg p2p.Message)

	// StateNotifier notifies the server peer state changes.
	StateNotifier StateNotifier
}

// normalizeAddress returns addr with the passed default port appended if
// there is not already a port specified.
func normalizeAddress(addr, defaultPort string) string {
	_, _, err := net.SplitHostPort(addr)
	if err != nil {
		return net.JoinHostPort(addr, defaultPort)
	}
	return addr
}
