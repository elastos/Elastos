package net

import (
	"errors"
	"time"
	"fmt"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MinConnections     = 3
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 30
)

type peerMsg struct {
	peer     *Peer
	inbound  bool
	connDone chan struct{}
}

// Handle the message creation, allocation etc.
type MessageHandler interface {
	// A handshake message received
	OnHandshake(v *msg.Version) error

	// Create a message instance by the given cmd parameter
	MakeMessage(cmd string) (p2p.Message, error)

	// VerAck message received from a connected peer
	// which means the connected peer is established
	OnPeerEstablish(*Peer)

	// Handle messages received from the connected peer
	HandleMessage(*Peer, p2p.Message) error
}

type PeerManager struct {
	magic           uint32
	maxMsgSize      uint32
	seeds           []string
	connectionQueue chan peerMsg
	disconnectQueue chan *Peer

	*Peers
	handler func() MessageHandler
	am      *AddrManager
	cm      *ConnManager
}

func NewPeerManager(magic, maxMsgSize uint32, seeds []string, minOutbound, maxConnections int, localPeer *Peer) *PeerManager {
	// Initiate PeerManager
	pm := new(PeerManager)
	pm.magic = magic
	pm.maxMsgSize = maxMsgSize
	pm.seeds = seeds
	pm.connectionQueue = make(chan peerMsg, 1)
	pm.disconnectQueue = make(chan *Peer, 1)
	pm.Peers = newPeers(localPeer)
	pm.am = newAddrManager(minOutbound)
	pm.cm = newConnManager(localPeer, maxConnections, pm)
	return pm
}

func (pm *PeerManager) SetMessageHandler(messageHandler func() MessageHandler) {
	pm.handler = messageHandler
}

func (pm *PeerManager) Start() {
	log.Info("PeerManager start")
	go pm.peersHandler()
	go pm.keepConnections()
	go pm.am.monitorAddresses()
	go pm.cm.listenConnection()
	go pm.cm.monitorConnections()
}

func (pm *PeerManager) OnConnection(msg connMsg) {
	// Create peer connection message
	doneChan := make(chan struct{})
	pm.connectionQueue <- peerMsg{
		connDone: doneChan,
		peer:     NewPeer(pm.magic, pm.maxMsgSize, msg.conn),
	}
	<-doneChan
}

// peersHandler handle peers from inbound/outbound and disconnected peers.
// This method will help to finish peers handshake progress and quit progress.
func (pm *PeerManager) peersHandler() {
	for {
		select {
		case msg := <-pm.connectionQueue:
			// Peers come from this queue are connected peers
			pm.handshake(msg.peer, msg.inbound, msg.connDone)

		case peer := <-pm.disconnectQueue:
			// Peers come from this queue are disconnected
			pm.PeerDisconnected(peer)
		}
	}
}

func (pm *PeerManager) handshake(peer *Peer, inbound bool, connDone chan struct{}) {
	// Create message handler instance
	handler := pm.handler()

	// doneChan notify to finish current handshake
	doneChan := make(chan struct{})

	// Set handshake handler
	peer.SetPeerHandler(PeerHandler{
		// New peer can only send handshake messages
		MakeMessage: func(cmd string) (p2p.Message, error) {
			switch cmd {
			case p2p.CmdVersion:
				return new(msg.Version), nil
			case p2p.CmdVerAck:
				return new(msg.VerAck), nil
			default:
				peer.Disconnect()
				return nil, fmt.Errorf("none handshake message [%s] received from new peer", cmd)
			}
		},

		// Handle handshake messages
		HandleMessage: func(peer *Peer, message p2p.Message) error {
			switch m := message.(type) {
			case *msg.Version:
				// Peer not in handshake state
				if peer.State() != p2p.INIT && peer.State() != p2p.HAND {
					peer.Disconnect()
					return fmt.Errorf("peer handshake with unknown state %d", peer.State())
				}
				// Callback handshake message first
				if err := handler.OnHandshake(m); err != nil {
					peer.Disconnect()
					return err
				}

				// Check if handshake with itself
				if m.Nonce == pm.Local().ID() {
					peer.Disconnect()
					return errors.New("Peer handshake with itself")
				}

				// If peer already connected, disconnect previous peer
				if oldPeer, ok := pm.RemovePeer(m.Nonce); ok {
					log.Warnf("Peer %d reconnect", m.Nonce)
					oldPeer.Disconnect()
				}

				// Set peer info with version message
				peer.SetInfo(m)

				if inbound {
					// Replay inbound handshake
					peer.SetState(p2p.HANDSHAKE)
					peer.Send(pm.Local().NewVersionMsg())

				} else {
					// Finish outbound handshake
					peer.SetState(p2p.HANDSHAKED)
					peer.Send(new(msg.VerAck))
				}

			case *msg.VerAck:
				// Peer not in handshake state
				if peer.State() != p2p.HANDSHAKE && peer.State() != p2p.HANDSHAKED {
					peer.Disconnect()
					return fmt.Errorf("peer handshake with unknown state %d", peer.State())
				}

				// Finish inbound handshake
				if inbound {
					peer.Send(new(msg.VerAck))
				}

				// Mark peer as establish
				peer.SetState(p2p.ESTABLISH)

				// Notify peer establish
				handler.OnPeerEstablish(peer)

				// Update peer's message handler
				peer.SetPeerHandler(pm.NewPeerHandler(handler))

				// Get more addresses
				if pm.am.NeedMoreAddresses() {
					peer.Send(new(msg.GetAddr))
				}

				// Notify handshake finished
				doneChan <- struct{}{}
			}
			return nil
		},

		// Peer disconnected
		OnDisconnected: func(peer *Peer) {
			pm.disconnectQueue <- peer
		},
	})

	// Start protocol
	if inbound {
		// Start inbound handshake
		peer.SetState(p2p.INIT)
	} else {
		// Start outbound handshake
		peer.SetState(p2p.HAND)
		peer.Send(pm.Local().NewVersionMsg())
	}

	peer.Start()

	// Wait for handshake progress finish or timeout
	timer := time.NewTimer(time.Second * HandshakeTimeout)

	select {
	case <-doneChan:
		// Stop timeout timer
		timer.Stop()

		// Add peer to neighbor list
		pm.PeerConnected(peer)

	case <-timer.C:
		// Disconnect peer for handshake timeout
		peer.Disconnect()
	}

	// Release connection
	connDone <- struct{}{}
}

func (pm *PeerManager) PeerConnected(peer *Peer) {
	log.Trace("PeerManager add connected peer:", peer)
	// Add peer to list
	pm.Peers.AddPeer(peer)
	// Mark addr as connected
	addr := peer.Addr()
	pm.am.AddressConnected(addr)
	pm.cm.PeerConnected(addr.String(), peer.conn)
}

func (pm *PeerManager) PeerDisconnected(peer *Peer) {
	if peer, ok := pm.Peers.RemovePeer(peer.ID()); ok {
		log.Trace("PeerManager peer disconnected:", peer)
		na := peer.Addr()
		addr := na.String()
		pm.am.AddressDisconnect(na)
		pm.cm.PeerDisconnected(addr)
		peer.Disconnect()
	}
}

func (pm *PeerManager) KnownAddresses() []p2p.NetAddress {
	return pm.am.KnowAddresses()
}

func (pm *PeerManager) keepConnections() {
	for {
		// connect seeds first
		if pm.PeersCount() < MinConnections {
			for _, seed := range pm.seeds {
				addr, err := pm.cm.ResolveAddr(seed)
				if err != nil {
					continue
				}

				if pm.cm.IsConnected(addr) {
					log.Debugf("Seed %s already connected", seed)
					continue
				}
				pm.cm.Connect(addr)
			}

		} else if pm.PeersCount() < pm.am.minOutbound {
			for _, addr := range pm.am.GetOutboundAddresses() {
				pm.cm.Connect(addr.String())
			}
		}

		// request more addresses
		if pm.am.NeedMoreAddresses() {
			pm.Broadcast(new(msg.GetAddr))
		}

		time.Sleep(time.Second * InfoUpdateDuration)
	}
}

func (pm *PeerManager) NewPeerHandler(handler MessageHandler) PeerHandler {
	return PeerHandler{
		MakeMessage: func(cmd string) (p2p.Message, error) {
			switch cmd {
			case p2p.CmdGetAddr:
				return new(msg.GetAddr), nil
			case p2p.CmdAddr:
				return new(msg.Addr), nil
			default:
				return handler.MakeMessage(cmd)
			}
		},

		HandleMessage: func(peer *Peer, message p2p.Message) error {
			switch m := message.(type) {
			case *msg.GetAddr:
				peer.Send(msg.NewAddr(pm.am.RandGetAddresses()))

			case *msg.Addr:
				for _, addr := range m.AddrList {
					// Skip local peer
					if addr.ID == pm.Local().ID() {
						continue
					}
					// Skip peer already connected
					if pm.EstablishedPeer(addr.ID) {
						continue
					}
					// Skip invalid port
					if addr.Port == 0 {
						continue
					}
					// Save to address list
					pm.am.AddOrUpdateAddress(&addr)
				}

			default:
				return handler.HandleMessage(peer, m)
			}

			return nil
		},

		OnDisconnected: func(peer *Peer) {
			pm.disconnectQueue <- peer
		},
	}
}
