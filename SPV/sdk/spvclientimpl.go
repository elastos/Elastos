package sdk

import (
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"github.com/elastos/Elastos.ELA/core"
)

type SPVClientImpl struct {
	p2pClient         P2PClient
	spvMessageHandler func() SPVMessageHandler
}

func NewSPVClientImpl(magic uint32, clientId uint64, seeds []string, minOutbound, maxConnections int) (*SPVClientImpl, error) {
	// Initialize P2P client
	p2pClient, err := GetP2PClient(magic, MaxMsgSize, clientId, seeds, 0, minOutbound, maxConnections)
	if err != nil {
		return nil, err
	}

	client := &SPVClientImpl{p2pClient: p2pClient}
	p2pClient.PeerManager().SetMessageHandler(client.newSpvHandler)

	return client, nil
}

func (client *SPVClientImpl) SetMessageHandler(spvMessageHandler func() SPVMessageHandler) {
	client.spvMessageHandler = spvMessageHandler
}

func (client *SPVClientImpl) Start() {
	client.p2pClient.Start()
}

func (client *SPVClientImpl) PeerManager() *net.PeerManager {
	return client.p2pClient.PeerManager()
}

func (client *SPVClientImpl) newSpvHandler() net.MessageHandler {
	return &spvHandler{
		peerManager:       client.PeerManager(),
		spvMessageHandler: client.spvMessageHandler(),
	}
}

type spvHandler struct {
	peerManager       *net.PeerManager
	spvMessageHandler SPVMessageHandler
}

// Filter peer handshake according to the SPV protocol
func (h *spvHandler) OnHandshake(v *msg.Version) error {
	//if v.Version < ProtocolVersion {
	//	return fmt.Errorf("To support SPV protocol, peer version must greater than ", ProtocolVersion)
	//}

	if v.Services/OpenService&1 == 0 {
		return errors.New("SPV service not enabled on connected peer")
	}

	return nil
}

func (h *spvHandler) MakeMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdPing:
		message = new(msg.Ping)
	case p2p.CmdPong:
		message = new(msg.Pong)
	case p2p.CmdInv:
		message = new(msg.Inventory)
	case p2p.CmdTx:
		message = msg.NewTx(new(core.Transaction))
	case p2p.CmdMerkleBlock:
		message = msg.NewMerkleBlock(new(core.Header))
	case p2p.CmdNotFound:
		message = new(msg.NotFound)
	case p2p.CmdReject:
		message = new(msg.Reject)
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}

func (h *spvHandler) HandleMessage(peer *net.Peer, message p2p.Message) error {
	switch message := message.(type) {
	case *msg.Ping:
		return h.OnPing(peer, message)
	case *msg.Pong:
		return h.OnPong(peer, message)
	case *msg.Inventory:
		return h.spvMessageHandler.OnInventory(peer, message)
	case *msg.MerkleBlock:
		return h.spvMessageHandler.OnMerkleBlock(peer, message)
	case *msg.Tx:
		return h.spvMessageHandler.OnTx(peer, message)
	case *msg.NotFound:
		return h.spvMessageHandler.OnNotFound(peer, message)
	case *msg.Reject:
		return h.spvMessageHandler.OnReject(peer, message)
	default:
		return errors.New("handle message unknown type")
	}
}

func (h *spvHandler) OnPeerEstablish(peer *net.Peer) {
	h.spvMessageHandler.OnPeerEstablish(peer)

	// Start heartbeat
	go h.heartBeat(peer)
}

func (h *spvHandler) OnPing(peer *net.Peer, p *msg.Ping) error {
	peer.SetHeight(p.Nonce)
	// Return pong message to peer
	peer.Send(msg.NewPong(uint32(h.peerManager.Local().Height())))
	return nil
}

func (h *spvHandler) OnPong(peer *net.Peer, p *msg.Pong) error {
	peer.SetHeight(p.Nonce)
	return nil
}

func (h *spvHandler) heartBeat(peer *net.Peer) {
	ticker := time.NewTicker(time.Second * net.InfoUpdateDuration)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			// Disconnect peer if keep alive timeout
			if time.Now().After(peer.LastActive().Add(time.Second * net.InfoUpdateDuration * net.KeepAliveTimeout)) {
				peer.Disconnect()
				return
			}

			// Send ping message to peer
			if peer.State() == p2p.ESTABLISH {
				peer.Send(msg.NewPing(uint32(h.peerManager.Local().Height())))
			}
		}
	}
}
