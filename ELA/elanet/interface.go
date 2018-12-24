package elanet

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/blockchain/interfaces"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	svr "github.com/elastos/Elastos.ELA/p2p/server"
)

type Config struct {
	Chain        *blockchain.BlockChain
	ChainParams  *config.Params
	Versions     interfaces.HeightVersions
	TxMemPool    *mempool.TxPool
	BlockMemPool *mempool.BlockPool
}

type Server interface {
	svr.IServer

	// NewPeer adds a new peer that has already been connected to the server.
	NewPeer(p svr.IPeer)

	// DonePeer removes a peer that has already been connected to the server by ip.
	DonePeer(p svr.IPeer)

	// RelayInventory relays the passed inventory vector to all connected peers
	// that are not already known to have it.
	RelayInventory(invVect *msg.InvVect, data interface{})

	// IsCurrent returns whether or not the sync manager believes it is synced
	// with the connected peers.
	IsCurrent() bool
}
