package net

import (
	"errors"
	"fmt"
	"time"
	"strings"
	"net"
	"strconv"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	MinConnCount       = 4
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 3
	MaxOutboundCount   = 6
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
	magic       uint32
	*Peers
	MessageHandler
	addrManager *AddrManager
	connManager *ConnManager
}

func InitPeerManager(magic uint32, localPeer *Peer, seeds []string) *PeerManager {
	// Initiate PeerManager
	pm := new(PeerManager)
	pm.magic = magic
	pm.Peers = newPeers(localPeer)
	pm.addrManager = newAddrManager(seeds)
	pm.connManager = newConnManager(pm)
	return pm
}

func (pm *PeerManager) SetMessageHandler(msgHandler MessageHandler) {
	pm.MessageHandler = msgHandler
}

func (pm *PeerManager) Start() {
	log.Info("PeerManager start")
	go pm.keepConnections()
	go pm.listenConnection()
}

func (pm *PeerManager) NewPeer(conn net.Conn) *Peer {
	peer := new(Peer)
	peer.conn = conn
	peer.ip16, peer.port = addrFromConn(conn)
	peer.msgHelper = p2p.NewMsgHelper(pm.magic, conn, peer)
	peer.handler = pm
	return peer
}

func addrFromConn(conn net.Conn) ([16]byte, uint16) {
	addr := conn.RemoteAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	port, _ := strconv.ParseUint(string([]byte(addr)[portIndex+1:]), 10, 16)
	ip := net.ParseIP(string([]byte(addr)[:portIndex])).To16()

	ip16 := [16]byte{}
	copy(ip16[:], ip[:])

	return ip16, uint16(port)
}

func (pm *PeerManager) NeedMorePeers() bool {
	return pm.PeersCount() < MinConnCount
}

func (pm *PeerManager) ConnectPeer(addr string) {
	pm.connManager.Connect(addr)
}

func (pm *PeerManager) OnPeerConnected(conn net.Conn) {
	// Start read msg from remote peer
	remote := pm.NewPeer(conn)
	remote.SetState(p2p.HAND)
	go remote.Read()

	// Send version message to remote peer
	go remote.Send(pm.local.NewVersionMsg())
}

func (pm *PeerManager) AddConnectedPeer(peer *Peer) {
	log.Trace("PeerManager add connected peer:", peer)
	// Add peer to list
	pm.Peers.AddPeer(peer)

	addr := peer.Addr().String()

	// Remove addr from connecting list
	pm.connManager.removeAddrFromConnectingList(addr)

	// Mark addr as connected
	pm.addrManager.AddAddr(addr)
}

func (pm *PeerManager) OnDisconnected(peer *Peer) {
	if peer == nil {
		return
	}
	log.Trace("PeerManager peer disconnected:", peer.String())
	peer, ok := pm.RemovePeer(peer.ID())
	if ok {
		peer.Disconnect()
		addr := peer.Addr().String()
		pm.connManager.removeAddrFromConnectingList(addr)
		pm.addrManager.DisconnectedAddr(addr)
	}
}

func (pm *PeerManager) OnDiscardAddr(addr string) {
	pm.addrManager.DiscardAddr(addr)
}

func (pm *PeerManager) RandAddrs() []msg.Addr {
	peers := pm.ConnectedPeers()

	log.Info("Rand peer addrs, connected peers:", peers)
	count := len(peers)
	if count > MaxOutboundCount {
		count = MaxOutboundCount
	}

	addrs := make([]msg.Addr, count)
	for count > 0 {
		count--
		addrs = append(addrs, *peers[count].Addr())
	}

	return addrs
}

func (pm *PeerManager) connectPeers() {
	if pm.NeedMorePeers() {
		addrs := pm.addrManager.GetIdleAddrs(MaxOutboundCount)
		for _, addr := range addrs {
			go pm.ConnectPeer(addr)
		}
	}
}

func (pm *PeerManager) keepConnections() {
	pm.connectPeers()

	ticker := time.NewTicker(time.Second * InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {
		pm.connectPeers()
	}
}

func (pm *PeerManager) listenConnection() {
	listener, err := net.Listen("tcp", fmt.Sprint(":", pm.Local().Port()))
	if err != nil {
		fmt.Println("Start peer listening err, ", err.Error())
		return
	}
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting ", err.Error())
			continue
		}
		fmt.Printf("New peer connection accepted, remote: %s local: %s\n", conn.RemoteAddr(), conn.LocalAddr())

		peer := pm.NewPeer(conn)
		go peer.Read()
	}
}

func (pm *PeerManager) MakeMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case "version":
		message = new(msg.Version)
	case "verack":
		message = new(msg.VerAck)
	case "getaddr":
		message = new(msg.AddrsReq)
	case "addr":
		message = new(msg.Addrs)
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
	case *msg.AddrsReq:
		return pm.OnAddrsReq(peer, message)
	case *msg.Addrs:
		return pm.OnAddrs(peer, message)
	default:
		return pm.MessageHandler.HandleMessage(peer, message)
	}
}

func (pm *PeerManager) OnVersion(peer *Peer, v *msg.Version) error {
	// Check if handshake with itself
	if v.Nonce == pm.Local().ID() {
		log.Error("SPV disconnect peer, peer handshake with itself")
		pm.OnDisconnected(peer)
		pm.OnDiscardAddr(peer.Addr().String())
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

	log.Info("Is known peer:", ok)

	// Set peer info with version message
	peer.SetInfo(v)

	// Handle peer handshake
	if err := pm.MessageHandler.OnHandshake(v); err != nil {
		pm.OnDisconnected(peer)
		return err
	}

	var message p2p.Message
	if peer.State() == p2p.INIT {
		peer.SetState(p2p.HANDSHAKE)
		message = pm.local.NewVersionMsg()
	} else if peer.State() == p2p.HAND {
		peer.SetState(p2p.HANDSHAKED)
		message = new(msg.VerAck)
	}

	go peer.Send(message)

	return nil
}

func (pm *PeerManager) OnVerAck(peer *Peer, va *msg.VerAck) error {
	if peer.State() != p2p.HANDSHAKE && peer.State() != p2p.HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == p2p.HANDSHAKE {
		go peer.Send(va)
	}

	peer.SetState(p2p.ESTABLISH)

	// Add to connected peer
	pm.AddConnectedPeer(peer)

	// Notify peer connected
	pm.MessageHandler.OnPeerEstablish(peer)

	if pm.NeedMorePeers() {
		go peer.Send(new(msg.AddrsReq))
	}

	return nil
}

func (pm *PeerManager) OnAddrs(peer *Peer, addrs *msg.Addrs) error {
	for _, addr := range addrs.Addrs {
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
		// Handle new address
		if pm.NeedMorePeers() {
			pm.ConnectPeer(addr.String())
		}
	}

	return nil
}

func (pm *PeerManager) OnAddrsReq(peer *Peer, req *msg.AddrsReq) error {
	addrs := pm.RandAddrs()
	go peer.Send(msg.NewAddrs(addrs))
	return nil
}
