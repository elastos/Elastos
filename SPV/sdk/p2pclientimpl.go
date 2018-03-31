package sdk

import (
	"errors"
	"fmt"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/p2p"
)

type P2PClientImpl struct {
	peerManager *p2p.PeerManager
}

func NewP2PClientImpl(magic uint32, clientId uint64, seeds []string) (*P2PClientImpl, error) {
	// Initialize local peer
	local := new(p2p.Peer)
	local.SetID(clientId)
	local.SetVersion(ProtocolVersion)
	local.SetPort(SPVClientPort)

	if magic == 0 {
		return nil, errors.New("Magic number has not been set ")
	}
	// Set Magic number of the P2P network
	p2p.Magic = magic

	if len(seeds) == 0 {
		return nil, errors.New("Seeds list is empty ")
	}
	// Initialize peer manager
	peerManager := p2p.InitPeerManager(local, toSPVAddr(seeds))

	// Set default methods to prevent nil exception
	peerManager.OnHandleVersion = handleVersion
	peerManager.OnMakeMessage = OnMakeMessage
	peerManager.OnPeerConnected = OnPeerConnected
	peerManager.OnHandleMessage = OnHandleMessage

	return &P2PClientImpl{
		peerManager: peerManager,
	}, nil
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

// Filter peer handshake according to the SPV protocol
func handleVersion(v *p2p.Version) error {
	if v.Version < ProtocolVersion {
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", ProtocolVersion))
	}

	if v.Services/ServiveSPV&1 == 0 {
		return errors.New("SPV service not enabled on connected peer")
	}

	return nil
}

func OnMakeMessage(cmd string) (p2p.Message, error) {
	return nil, errors.New("OnMakeMessage method not initialized")
}

func OnPeerConnected(peer *p2p.Peer) {}

func OnHandleMessage(peer *p2p.Peer, msg p2p.Message) error {
	return errors.New("OnHandleMessage method not initialized")
}

func (client *P2PClientImpl) OnPeerConnected(callback func(peer *p2p.Peer)) {
	client.peerManager.OnPeerConnected = callback
}

func (client *P2PClientImpl) OnMakeMessage(callback func(cmd string) (p2p.Message, error)) {
	client.peerManager.OnMakeMessage = callback
}

func (client *P2PClientImpl) OnHandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error) {
	client.peerManager.OnHandleMessage = callback
}

func (client *P2PClientImpl) PeerManager() *p2p.PeerManager {
	return client.peerManager
}
