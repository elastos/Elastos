package _interface

import (
	"github.com/elastos/Elastos.ELA.SPV/net"
)

type P2PClientImpl struct {
	magic      uint32
	maxMsgSize uint32

	seeds          []string
	minOutbound    int
	maxConnections int
	pm             *net.PeerManager
}

func NewP2PClientImpl(magic, maxMsgSize uint32, seeds []string, minOutbound, maxConnections int) *P2PClientImpl {
	return &P2PClientImpl{
		magic:          magic,
		maxMsgSize:     maxMsgSize,
		seeds:          seeds,
		minOutbound:    minOutbound,
		maxConnections: maxConnections,
	}
}

func (c *P2PClientImpl) InitLocalPeer(initLocal func(peer *net.Peer)) {
	// Create peer manager of the P2P network
	local := new(net.Peer)
	initLocal(local)
	c.pm = net.NewPeerManager(c.magic, c.maxMsgSize, c.seeds, c.minOutbound, c.maxConnections, local)
}

func (c *P2PClientImpl) SetMessageHandler(messageHandler func() net.MessageHandler) {
	c.pm.SetMessageHandler(messageHandler)
}

func (c *P2PClientImpl) Start() {
	c.pm.Start()
}

func (c *P2PClientImpl) PeerManager() *net.PeerManager {
	return c.pm
}
