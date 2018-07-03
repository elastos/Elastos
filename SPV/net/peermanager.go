package net

import (
	"errors"
	"net"
	"strings"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MinConnections     = 3
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 3
)

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
	magic      uint32
	maxMsgSize uint32
	seeds      []string
	*Peers
	MessageHandler
	am *AddrManager
	cm *ConnManager
}

func InitPeerManager(magic, maxMsgSize uint32, seeds []string, minOutbound, maxConnections int, localPeer *Peer) *PeerManager {
	// Initiate PeerManager
	pm := new(PeerManager)
	pm.magic = magic
	pm.maxMsgSize = maxMsgSize
	pm.seeds = seeds
	pm.Peers = newPeers(localPeer)
	pm.am = newAddrManager(minOutbound)
	pm.cm = newConnManager(localPeer, maxConnections, pm)
	return pm
}

func (pm *PeerManager) SetMessageHandler(msgHandler MessageHandler) {
	pm.MessageHandler = msgHandler
}

func (pm *PeerManager) Start() {
	log.Info("PeerManager start")
	go pm.keepConnections()
	go pm.am.monitorAddresses()
	go pm.cm.listenConnection()
	go pm.cm.monitorConnections()
}

func (pm *PeerManager) NewPeer(conn net.Conn) *Peer {
	peer := new(Peer)
	peer.conn = conn
	copy(peer.ip16[:], getIp(conn))
	peer.msgHelper = p2p.NewMsgHelper(pm.magic, pm.maxMsgSize, conn, peer)
	peer.handler = pm
	return peer
}

func getIp(conn net.Conn) []byte {
	addr := conn.RemoteAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	return net.ParseIP(string([]byte(addr)[:portIndex])).To16()
}

func (pm *PeerManager) OnOutbound(conn net.Conn) {
	// Start read msg from remote peer
	remote := pm.NewPeer(conn)
	remote.SetState(p2p.HAND)
	remote.Read()

	// Send version message to remote peer
	remote.Send(pm.local.NewVersionMsg())
}

func (pm *PeerManager) OnInbound(conn net.Conn) {
	peer := pm.NewPeer(conn)
	peer.Read()
}

func (pm *PeerManager) AddConnectedPeer(peer *Peer) {
	log.Trace("PeerManager add connected peer:", peer)
	// Add peer to list
	pm.Peers.AddPeer(peer)
	// Mark addr as connected
	addr := peer.Addr()
	pm.am.AddressConnected(addr)
	pm.cm.PeerConnected(addr.String(), peer.conn)
}

func (pm *PeerManager) OnDisconnected(peer *Peer) {
	if peer == nil {
		return
	}
	log.Trace("PeerManager peer disconnected:", peer.String())
	peer, ok := pm.RemovePeer(peer.ID())
	if ok {
		na := peer.Addr()
		addr := na.String()
		pm.am.AddressDisconnect(na)
		pm.cm.PeerDisconnected(addr)
	}
}

func (pm *PeerManager) KnownAddresses() []p2p.NetAddress {
	nas := make([]p2p.NetAddress, 0, len(pm.am.addrList))
	for _, ka := range pm.am.addrList {
		nas = append(nas, ka.NetAddress)
	}
	return nas
}

func (pm *PeerManager) connectPeers() {
	// connect seeds first
	if pm.PeersCount() < MinConnections {
		for _, addr := range pm.seeds {
			if pm.cm.isConnected(addr) {
				continue
			}
			go pm.cm.Connect(addr)
		}
	}

	// connect more peers
	if pm.PeersCount() < pm.am.minOutbound {
		for _, addr := range pm.am.GetOutboundAddresses(pm.cm) {
			go pm.cm.Connect(addr.String())
		}
	}

	// request more addresses
	if pm.am.NeedMoreAddresses() {
		go pm.Broadcast(new(msg.GetAddr))
	}
}

func (pm *PeerManager) keepConnections() {
	ticker := time.NewTicker(time.Second * InfoUpdateDuration)
	defer ticker.Stop()
	for {
		pm.connectPeers()
		<-ticker.C
	}
}

func (pm *PeerManager) MakeMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)
	case p2p.CmdVerAck:
		message = new(msg.VerAck)
	case p2p.CmdGetAddr:
		message = new(msg.GetAddr)
	case p2p.CmdAddr:
		message = new(msg.Addr)
	default:
		return pm.MessageHandler.MakeMessage(cmd)
	}

	return message, nil
}

func (pm *PeerManager) HandleMessage(peer *Peer, message p2p.Message) error {
	switch message := message.(type) {
	case *msg.Version:
		return pm.OnVersion(peer, message)
	case *msg.VerAck:
		return pm.OnVerAck(peer, message)
	case *msg.GetAddr:
		return pm.OnGetAddr(peer, message)
	case *msg.Addr:
		return pm.OnAddr(peer, message)
	default:
		return pm.MessageHandler.HandleMessage(peer, message)
	}
}

func (pm *PeerManager) OnVersion(peer *Peer, v *msg.Version) error {
	// Check if handshake with itself
	if v.Nonce == pm.Local().ID() {
		log.Error("SPV disconnect peer, peer handshake with itself")
		peer.Disconnect()
		return errors.New("Peer handshake with itself")
	}

	if peer.State() != p2p.INIT && peer.State() != p2p.HAND {
		log.Error("Unknow status to received version")
		return errors.New("Unknow status to received version")
	}

	// Remove duplicate peer connection
	knownPeer, ok := pm.RemovePeer(v.Nonce)
	if ok {
		log.Trace("Reconnect peer ", v.Nonce)
		knownPeer.Disconnect()
	}

	// Handle peer handshake
	if err := pm.MessageHandler.OnHandshake(v); err != nil {
		peer.Disconnect()
		return err
	}

	// Set peer info with version message
	peer.SetInfo(v)

	var message p2p.Message
	if peer.State() == p2p.INIT {
		peer.SetState(p2p.HANDSHAKE)
		message = pm.local.NewVersionMsg()
	} else if peer.State() == p2p.HAND {
		peer.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}

	peer.Send(message)

	return nil
}

func (pm *PeerManager) OnVerAck(peer *Peer, va *msg.VerAck) error {
	if peer.State() != p2p.HANDSHAKE && peer.State() != p2p.HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == p2p.HANDSHAKE {
		peer.Send(va)
	}

	peer.SetState(p2p.ESTABLISH)

	// Add to connected peer
	pm.AddConnectedPeer(peer)

	// Notify peer connected
	pm.MessageHandler.OnPeerEstablish(peer)

	if pm.am.NeedMoreAddresses() {
		peer.Send(new(msg.GetAddr))
	}

	return nil
}

func (pm *PeerManager) OnAddr(peer *Peer, addr *msg.Addr) error {
	for _, addr := range addr.AddrList {
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

	return nil
}

func (pm *PeerManager) OnGetAddr(peer *Peer, req *msg.GetAddr) error {
	peer.Send(msg.NewAddr(pm.am.RandGetAddresses()))
	return nil
}
