package _interface

import (
	"SPVWallet/p2p"
)

type P2PClientImpl struct {
	id    uint64
	magic uint32
	port  uint16
	seeds []string
	pm    *p2p.PeerManager
}

func (client *P2PClientImpl) Start() {
	// Set Magic number of the P2P network
	p2p.Magic = client.magic
	// Create peer manager of the P2P network
	client.pm = p2p.NewPeerManager(client.id, client.port, client.seeds)

	client.pm.Start()
}

func (client *P2PClientImpl) RegisterCallback(callback func(peer *p2p.Peer, msg p2p.Message)) {
	p2p.RegisterCallback(callback)
}

func (client *P2PClientImpl) PeerManager() *p2p.PeerManager {
	return client.pm
}
