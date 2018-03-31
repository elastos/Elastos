package impl

import (
	"errors"
	"fmt"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/p2p"
)

type P2PClient struct {
	magic       uint32
	serverPort  uint32
	localPeer   *p2p.Peer
	peerManager *p2p.PeerManager
}

func NewP2PClient(magic, serverPort uint32, local *p2p.Peer) *P2PClient {
	return &P2PClient{
		magic:      magic,
		serverPort: serverPort,
		localPeer:  local,
	}
}

func (client *P2PClient) Start(seeds []string) error {
	if client.magic == 0 {
		return errors.New("Magic number has not been set ")
	}
	// Set Magic number of the P2P network
	p2p.Magic = client.magic

	if client.localPeer == nil {
		return errors.New("Local peer can not be nil ")
	}

	if len(seeds) == 0 {
		return errors.New("Seeds list is empty ")
	}
	// Initialize peer manager
	client.peerManager = p2p.InitPeerManager(client.localPeer, client.toSPVAddr(seeds))

	// Start
	client.peerManager.Start()

	return nil
}

// Convert seed addresses to SPVServerPort according to the SPV protocol
func (client *P2PClient) toSPVAddr(seeds []string) []string {
	var addrs = make([]string, len(seeds))
	for i, seed := range seeds {
		portIndex := strings.LastIndex(seed, ":")
		if portIndex > 0 {
			addrs[i] = fmt.Sprint(string([]byte(seed)[:portIndex]), ":", client.serverPort)
		} else {
			addrs[i] = fmt.Sprint(seed, ":", client.serverPort)
		}
	}
	return addrs
}

func (client *P2PClient) OnHandleVersion(callback func(v *p2p.Version) error) {
	p2p.OnHandleVersion(callback)
}

func (client *P2PClient) OnPeerConnected(callback func(peer *p2p.Peer)) {
	p2p.OnPeerConnected(callback)
}

func (client *P2PClient) OnMakeMessage(callback func(cmd string) (p2p.Message, error)) {
	p2p.OnMakeMessage(callback)
}

func (client *P2PClient) OnHandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error) {
	p2p.OnHandleMessage(callback)
}

func (client *P2PClient) PeerManager() *p2p.PeerManager {
	return client.peerManager
}
