package server

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	svr "github.com/elastos/Elastos.ELA.Utility/p2p/server"
)

type Server interface {
	svr.IServer

	// NewPeer adds a new peer that has already been connected to the server.
	NewPeer(p svr.IPeer)

	// DonePeer removes a peer that has already been connected to the server by ip.
	DonePeer(p svr.IPeer)

	// RelayInventory relays the passed inventory vector to all connected peers
	// that are not already known to have it.
	RelayInventory(invVect *msg.InvVect, data interface{})
}
