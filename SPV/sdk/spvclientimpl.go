package sdk

import (
	"errors"
	"time"
	"fmt"

	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA/bloom"
	"github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type SPVClientImpl struct {
	p2pClient  P2PClient
	msgHandler SPVMessageHandler
}

func NewSPVClientImpl(magic uint32, clientId uint64, seeds []string) (*SPVClientImpl, error) {
	// Initialize P2P client
	p2pClient, err := GetP2PClient(magic, clientId, seeds)
	if err != nil {
		return nil, err
	}

	client := &SPVClientImpl{p2pClient: p2pClient}
	p2pClient.PeerManager().SetMessageHandler(client)

	return client, nil
}

func (client *SPVClientImpl) SetMessageHandler(handler SPVMessageHandler) {
	client.msgHandler = handler
}

func (client *SPVClientImpl) Start() {
	client.p2pClient.Start()
}

func (client *SPVClientImpl) PeerManager() *net.PeerManager {
	return client.p2pClient.PeerManager()
}

// Filter peer handshake according to the SPV protocol
func (client *SPVClientImpl) OnHandshake(v *msg.Version) error {
	if v.Version < ProtocolVersion {
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", ProtocolVersion))
	}

	if v.Services/ServiveSPV&1 == 0 {
		return errors.New("SPV service not enabled on connected peer")
	}

	return nil
}

func (client *SPVClientImpl) MakeMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case "ping":
		message = new(msg.Ping)
	case "pong":
		message = new(msg.Pong)
	case "inv":
		message = new(msg.Inventory)
	case "tx":
		message = new(core.Transaction)
	case "merkleblock":
		message = new(bloom.MerkleBlock)
	case "notfound":
		message = new(msg.NotFound)
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}

func (client *SPVClientImpl) HandleMessage(peer *net.Peer, message p2p.Message) error {
	switch msg := message.(type) {
	case *msg.Ping:
		return client.OnPing(peer, msg)
	case *msg.Pong:
		return client.OnPong(peer, msg)
	case *msg.Inventory:
		return client.msgHandler.OnInventory(peer, msg)
	case *bloom.MerkleBlock:
		return client.msgHandler.OnMerkleBlock(peer, msg)
	case *core.Transaction:
		return client.msgHandler.OnTx(peer, msg)
	case *msg.NotFound:
		return client.msgHandler.OnNotFound(peer, msg)
	default:
		return errors.New("handle message unknown type")
	}
}

func (client *SPVClientImpl) OnPeerEstablish(peer *net.Peer) {
	client.msgHandler.OnPeerEstablish(peer)
}

func (client *SPVClientImpl) OnPing(peer *net.Peer, p *msg.Ping) error {
	peer.SetHeight(p.Height)
	// Return pong message to peer
	go peer.Send(msg.NewPong(uint32(client.PeerManager().Local().Height())))
	return nil
}

func (client *SPVClientImpl) OnPong(peer *net.Peer, p *msg.Pong) error {
	peer.SetHeight(p.Height)
	return nil
}

func (client *SPVClientImpl) keepUpdate() {
	ticker := time.NewTicker(time.Second * net.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {

		// Update peers info
		for _, peer := range client.PeerManager().ConnectedPeers() {
			if peer.State() == p2p.ESTABLISH {

				// Disconnect inactive peer
				if peer.LastActive().Before(
					time.Now().Add(-time.Second * net.InfoUpdateDuration * net.KeepAliveTimeout)) {
					client.PeerManager().OnDisconnected(peer)
					continue
				}

				// Send ping message to peer
				go peer.Send(msg.NewPing(uint32(client.PeerManager().Local().Height())))
			}
		}
	}
}
