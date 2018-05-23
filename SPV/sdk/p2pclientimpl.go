package sdk

import (
	"errors"
	"fmt"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/net"
)

type P2PClientImpl struct {
	peerManager *net.PeerManager
}

func NewP2PClientImpl(magic uint32, clientId uint64, seeds []string, maxOutbound, maxConnections int) (*P2PClientImpl, error) {
	// Initialize local peer
	local := new(net.Peer)
	local.SetID(clientId)
	local.SetVersion(ProtocolVersion)
	local.SetPort(SPVClientPort)

	if magic == 0 {
		return nil, errors.New("Magic number has not been set ")
	}

	if len(seeds) == 0 {
		return nil, errors.New("Seeds list is empty ")
	}

	// Create client instance
	client := new(P2PClientImpl)

	// Initialize peer manager
	client.peerManager = net.InitPeerManager(magic, toSPVAddr(seeds), maxOutbound, maxConnections, local)

	return client, nil
}

func (client *P2PClientImpl) Start() {
	// Start
	client.peerManager.Start()
}

// Convert seed addresses to SPVServerPort according to the SPV protocol
func toSPVAddr(seeds []string) []string {
	var addrs = make([]string, len(seeds))
	for i, seed := range seeds {
		portIndex := strings.LastIndex(seed, ":")
		if portIndex > 0 {
			addrs[i] = fmt.Sprint(string([]byte(seed)[:portIndex]), ":", SPVServerPort)
		} else {
			addrs[i] = fmt.Sprint(seed, ":", SPVServerPort)
		}
	}
	return addrs
}

func (client *P2PClientImpl) PeerManager() *net.PeerManager {
	return client.peerManager
}
