package sdk

import (
	"errors"
	"github.com/elastos/Elastos.ELA.SPV/net"
)

type P2PClientImpl struct {
	peerManager *net.PeerManager
}

func NewP2PClientImpl(magic uint32, clientId uint64, seeds []string, port uint16, maxOutbound, maxConnections int) (*P2PClientImpl, error) {
	// Initialize local peer
	local := new(net.Peer)
	local.SetID(clientId)
	local.SetVersion(ProtocolVersion)
	local.SetPort(port)

	if magic == 0 {
		return nil, errors.New("Magic number has not been set ")
	}

	if len(seeds) == 0 {
		return nil, errors.New("Seeds list is empty ")
	}

	// Create client instance
	client := new(P2PClientImpl)

	// Initialize peer manager
	client.peerManager = net.InitPeerManager(magic, seeds, maxOutbound, maxConnections, local)

	return client, nil
}

func (client *P2PClientImpl) Start() {
	// Start
	client.peerManager.Start()
}

func (client *P2PClientImpl) PeerManager() *net.PeerManager {
	return client.peerManager
}
