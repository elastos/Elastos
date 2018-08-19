package net

import (
	"errors"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
	"net"
)

const (
	MinConnections     = 3
	InfoUpdateDuration = time.Second * 5
)

// P2P network config
type ServerPeerConfig struct {
	Magic          uint32
	Version        uint32
	PeerId         uint64
	Port           uint16
	Seeds          []string
	MinOutbound    int
	MaxConnections int
}

// Handle the message creation, allocation etc.
type PeerManageConfig struct {
	// A handshake message received
	OnHandshake func(v *msg.Version) error

	// VerAck message received from a connected peer
	// which means the connected peer is established
	OnPeerEstablish func(*Peer)
}

type ServerPeer struct {
	Peer
	magic uint32
	seeds []string
	neighbors
	quitChan chan uint64
	am       *AddrManager
	cm       *ConnManager
	config   PeerManageConfig
}

func NewServerPeer(config ServerPeerConfig) *ServerPeer {
	// Initiate ServerPeer
	sp := new(ServerPeer)
	sp.id = config.PeerId
	sp.version = config.Version
	sp.port = config.Port
	sp.magic = config.Magic
	sp.seeds = config.Seeds
	sp.neighbors.Init()
	sp.quitChan = make(chan uint64, 1)
	sp.am = newAddrManager(config.MinOutbound)
	sp.cm = newConnManager(sp.port, config.MaxConnections)
	sp.cm.OnConnection = sp.OnConnection
	return sp
}

func (sp *ServerPeer) SetConfig(config PeerManageConfig) {
	sp.config = config
}

func (sp *ServerPeer) Start() {
	log.Info("ServerPeer start")
	go sp.keepConnections()
	go sp.peerQuitHandler()
	go sp.am.monitorAddresses()
	go sp.cm.listenConnection()
	go sp.cm.monitorConnections()
}

func (sp *ServerPeer) OnConnection(conn net.Conn, inbound bool) {
	// Start handshake
	doneChan := make(chan struct{})
	go func() {
		sp.handshake(NewPeer(sp.magic, conn), inbound, doneChan)
	}()
	<-doneChan
}

func (sp *ServerPeer) readVersionMsg(peer *Peer) error {
	message, err := peer.readMessage()
	if err != nil {
		return err
	}

	version, ok := message.(*msg.Version)
	if !ok {
		errMsg := "A version message must precede all others"
		log.Error(errMsg)

		reject := msg.NewReject(message.CMD(), msg.RejectMalformed, errMsg)
		return peer.writeMessage(reject)
	}

	if err := sp.config.OnHandshake(version); err != nil {
		return err
	}

	// Check if handshake with itself
	if version.Nonce == sp.ID() {
		peer.Disconnect()
		return errors.New("peer handshake with itself")
	}

	// If peer already connected, disconnect previous peer
	if oldPeer, ok := sp.DelNeighbor(version.Nonce); ok {
		log.Warnf("Peer %d reconnect", version.Nonce)
		oldPeer.Disconnect()
	}

	// Set peer info with version message
	peer.SetInfo(version)

	return nil
}

func (sp *ServerPeer) inboundProtocol(peer *Peer) error {
	if err := sp.readVersionMsg(peer); err != nil {
		return err
	}

	return peer.writeMessage(sp.NewVersionMsg())
}

func (sp *ServerPeer) outboundProtocol(peer *Peer) error {
	if err := peer.writeMessage(sp.NewVersionMsg()); err != nil {
		return err
	}

	return sp.readVersionMsg(peer)
}

func (sp *ServerPeer) handshake(peer *Peer, inbound bool, doneChan chan struct{}) {
	errChan := make(chan error)
	go func() {
		if inbound {
			errChan <- sp.inboundProtocol(peer)
		} else {
			errChan <- sp.outboundProtocol(peer)
		}
	}()

	select {
	case err := <-errChan:
		if err != nil {
			return
		}

	case <-time.After(HandshakeTimeout):
		// Disconnect peer for handshake timeout
		peer.Disconnect()

		// Notify handshake done
		doneChan <- struct{}{}
		return
	}

	// Wait for peer quit
	go func() {
		select {
		case <-peer.quit:
			sp.quitChan <- peer.id
		}
	}()

	// Add peer to neighbor list
	sp.AddToNeighbors(peer)

	// Notify handshake done
	doneChan <- struct{}{}

	// Update peer's message config
	peer.handleMessage = sp.baseMessageHandler()

	// Start peer
	peer.start()

	// Send our verack message now that the IO processing machinery has started.
	peer.QueueMessage(new(msg.VerAck), nil)
}

func (sp *ServerPeer) peerQuitHandler() {
	for peerId := range sp.quitChan {
		if peer, ok := sp.neighbors.DelNeighbor(peerId); ok {
			log.Trace("ServerPeer peer disconnected:", peer)
			na := peer.Addr()
			addr := na.String()
			sp.am.AddressDisconnect(na)
			sp.cm.PeerDisconnected(addr)
		}
	}
}

func (sp *ServerPeer) AddToNeighbors(peer *Peer) {
	log.Trace("ServerPeer add connected peer:", peer)
	// Add peer to list
	sp.AddNeighbor(peer)

	// Mark addr as connected
	addr := peer.Addr()
	sp.am.AddressConnected(addr)
	sp.cm.PeerConnected(addr.String(), peer.conn)
}

func (sp *ServerPeer) KnownAddresses() []p2p.NetAddress {
	return sp.am.KnowAddresses()
}

func (sp *ServerPeer) keepConnections() {
	for {
		// connect seeds first
		if sp.GetNeighborCount() < MinConnections {
			for _, seed := range sp.seeds {
				sp.cm.Connect(seed)
			}

		} else if sp.GetNeighborCount() < sp.am.minOutbound {
			for _, addr := range sp.am.GetOutboundAddresses() {
				sp.cm.Connect(addr.String())
			}
		}

		// request more addresses
		if sp.am.NeedMoreAddresses() {
			sp.Broadcast(new(msg.GetAddr))
		}

		time.Sleep(InfoUpdateDuration)
	}
}

func (sp *ServerPeer) baseMessageHandler() func(peer *Peer, message p2p.Message) {
	return func(peer *Peer, message p2p.Message) {
		switch m := message.(type) {
		case *msg.VerAck:
			// Notify peer establish
			sp.config.OnPeerEstablish(peer)

		case *msg.GetAddr:
			peer.QueueMessage(msg.NewAddr(sp.am.RandGetAddresses()), nil)

		case *msg.Addr:
			for _, addr := range m.AddrList {
				// Skip local peer
				if addr.ID == sp.ID() {
					continue
				}
				// Skip peer already connected
				if sp.IsNeighborPeer(addr.ID) {
					continue
				}
				// Skip invalid port
				if addr.Port == 0 {
					continue
				}
				// Save to address list
				sp.am.AddOrUpdateAddress(&addr)
			}
		}
	}
}

func (sp *ServerPeer) Broadcast(msg p2p.Message) {
	// Make a copy of neighbor peers list,
	// This can prevent mutex lock when peer.Send()
	// method fire a disconnect event.
	neighbors := sp.GetNeighborPeers()

	// Do broadcast
	go func() {
		for _, peer := range neighbors {

			// Skip disconnected peer
			if !peer.Connected() {
				continue
			}

			// Skip non relay peer
			if peer.Relay() == 0 {
				continue
			}

			peer.QueueMessage(msg, nil)
		}
	}()
}
