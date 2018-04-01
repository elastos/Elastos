package sdk

import (
	"errors"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/bloom"
	. "github.com/elastos/Elastos.ELA.SPV/common"
	tx "github.com/elastos/Elastos.ELA.SPV/core/transaction"
	"github.com/elastos/Elastos.ELA.SPV/p2p"
	"github.com/elastos/Elastos.ELA.SPV/msg"
)

type SPVClientImpl struct {
	p2p        P2PClient
	msgHandler SPVMessageHandler
}

func NewSPVClientImpl(magic uint32, clientId uint64, seeds []string) (*SPVClientImpl, error) {
	// Initialize P2P client
	p2p, err := GetP2PClient(magic, clientId, seeds)
	if err != nil {
		return nil, err
	}

	client := &SPVClientImpl{p2p: p2p}
	p2p.SetMessageHandler(client)

	return client, nil
}

func (client *SPVClientImpl) SetMessageHandler(handler SPVMessageHandler) {
	client.msgHandler = handler
}

func (client *SPVClientImpl) Start() {
	client.p2p.Start()
}

func (client *SPVClientImpl) PeerManager() *p2p.PeerManager {
	return client.p2p.PeerManager()
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
		message = new(msg.Txn)
	case "merkleblock":
		message = new(bloom.MerkleBlock)
	case "notfound":
		message = new(msg.NotFound)
	default:
		return nil, errors.New("Received unsupported message, CMD " + cmd)
	}
	return message, nil
}

func (client *SPVClientImpl) HandleMessage(peer *p2p.Peer, message p2p.Message) error {
	switch msg := message.(type) {
	case *msg.Ping:
		return client.OnPing(peer, msg)
	case *msg.Pong:
		return client.OnPong(peer, msg)
	case *msg.Inventory:
		return client.msgHandler.OnInventory(peer, msg)
	case *bloom.MerkleBlock:
		return client.msgHandler.OnMerkleBlock(peer, msg)
	case *msg.Txn:
		return client.msgHandler.OnTxn(peer, msg)
	case *msg.NotFound:
		return client.msgHandler.OnNotFound(peer, msg)
	default:
		return errors.New("handle message unknown type")
	}
}

func (client *SPVClientImpl) NewPing(height uint32) *msg.Ping {
	ping := new(msg.Ping)
	ping.Height = uint64(height)
	return ping
}

func (client *SPVClientImpl) NewPong(height uint32) *msg.Pong {
	pong := new(msg.Pong)
	pong.Height = uint64(height)
	return pong
}

func (client *SPVClientImpl) NewBlocksReq(locator []*Uint256, hashStop Uint256) *msg.BlocksReq {
	blocksReq := new(msg.BlocksReq)
	blocksReq.Count = uint32(len(locator))
	blocksReq.BlockLocator = locator
	blocksReq.HashStop = hashStop
	return blocksReq
}

func (client *SPVClientImpl) NewDataReq(invType uint8, hash Uint256) *msg.DataReq {
	dataReq := new(msg.DataReq)
	dataReq.Type = invType
	dataReq.Hash = hash
	return dataReq
}

func (client *SPVClientImpl) NewTxn(tx tx.Transaction) *msg.Txn {
	return &msg.Txn{Transaction: tx}
}

func (client *SPVClientImpl) OnPeerEstablish(peer *p2p.Peer) {
	client.msgHandler.OnPeerEstablish(peer)
}

func (client *SPVClientImpl) OnPing(peer *p2p.Peer, p *msg.Ping) error {
	peer.SetHeight(p.Height)
	// Return pong message to peer
	go peer.Send(client.NewPong(uint32(client.PeerManager().Local().Height())))
	return nil
}

func (client *SPVClientImpl) OnPong(peer *p2p.Peer, p *msg.Pong) error {
	peer.SetHeight(p.Height)
	return nil
}

func (client *SPVClientImpl) keepUpdate() {
	ticker := time.NewTicker(time.Second * p2p.InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {

		// Update peers info
		for _, peer := range client.PeerManager().ConnectedPeers() {
			if peer.State() == p2p.ESTABLISH {

				// Disconnect inactive peer
				if peer.LastActive().Before(
					time.Now().Add(-time.Second * p2p.InfoUpdateDuration * p2p.KeepAliveTimeout)) {
					client.PeerManager().DisconnectPeer(peer)
					continue
				}

				// Send ping message to peer
				go peer.Send(client.NewPing(uint32(client.PeerManager().Local().Height())))
			}
		}
	}
}
