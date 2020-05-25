// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package elanet

import (
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/elanet/routes"
	"github.com/elastos/Elastos.ELA/mempool"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	svr "github.com/elastos/Elastos.ELA/p2p/server"
)

// Config is the parameters needed to create a Server instance.
type Config struct {
	// Chain is the BlockChain instance.
	Chain *blockchain.BlockChain

	// ChainParams is the initial parameters to start the blockchain.
	ChainParams *config.Params

	// PermanentPeers are the peers need to be connected permanently.
	PermanentPeers []string

	// TxMemPool is the transaction mempool.
	TxMemPool *mempool.TxPool

	// BlockMemPool is the block mempool uses by DPOS consensus.
	BlockMemPool *mempool.BlockPool

	// Routes is the DPOS network routes depends on the normal P2P network.
	Routes *routes.Routes
}

// Server represent the elanet server.
//
// The interface contract requires that all of these methods are safe for
// concurrent access.
type Server interface {
	svr.IServer

	// Services returns the service flags the server supports.
	Services() pact.ServiceFlag

	// NewPeer adds a new peer that has already been connected to the server.
	NewPeer(p svr.IPeer)(bool)

	// DonePeer removes a peer that has already been connected to the server by ip.
	DonePeer(p svr.IPeer)

	// RelayInventory relays the passed inventory vector to all connected peers
	// that are not already known to have it.
	RelayInventory(invVect *msg.InvVect, data interface{})

	// IsCurrent returns whether or not the sync manager believes it is synced
	// with the connected peers.
	IsCurrent() bool
}
