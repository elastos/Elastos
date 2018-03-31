package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/p2p"
)

type P2PClientImpl struct {
	magic uint32
	seeds []string
	pm    *p2p.PeerManager
}

func (client *P2PClientImpl) InitLocalPeer(initLocal func(peer *p2p.Peer)) {
	// Set Magic number of the P2P network
	p2p.Magic = client.magic
	// Create peer manager of the P2P network
	local := new(p2p.Peer)
	initLocal(local)
	client.pm = p2p.InitPeerManager(local, client.seeds)
}

func (client *P2PClientImpl) Start() {
	client.pm.Start()
}

func (client *P2PClientImpl) HandleVersion(callback func(v *p2p.Version) error) {
	client.pm.OnHandleVersion = callback
}

func (client *P2PClientImpl) PeerConnected(callback func(peer *p2p.Peer)) {
	client.pm.OnPeerConnected = callback
}

func (client *P2PClientImpl) MakeMessage(callback func(cmd string) (p2p.Message, error)) {
	client.pm.OnMakeMessage = callback
}

func (client *P2PClientImpl) HandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error) {
	client.pm.OnHandleMessage = callback
}

func (client *P2PClientImpl) PeerManager() *p2p.PeerManager {
	return client.pm
}
