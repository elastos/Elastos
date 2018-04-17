package p2p

import (
	"errors"
	"fmt"
	"net"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	. "github.com/elastos/Elastos.ELA.SPV/p2p/msg"
)

const (
	MinConnCount       = 4
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 3
	MaxOutboundCount   = 6
)

var pm *PeerManager

type PeerManager struct {
	*Peers
	addrManager *AddrManager
	connManager *ConnManager
	msgHandler  MessageHandler
}

func InitPeerManager(localPeer *Peer, seeds []string) *PeerManager {
	// Initiate PeerManager
	pm = new(PeerManager)
	pm.Peers = newPeers(localPeer)
	pm.addrManager = newAddrManager(seeds)
	pm.connManager = newConnManager(pm.OnDiscardAddr)
	return pm
}

func (pm *PeerManager) SetMessageHandler(msgHandler MessageHandler) {
	pm.msgHandler = msgHandler
}

func (pm *PeerManager) Start() {
	log.Info("PeerManager start")
	go pm.keepConnections()
	go pm.listenConnection()
}

func (pm *PeerManager) NeedMorePeers() bool {
	return pm.PeersCount() < MinConnCount
}

func (pm *PeerManager) ConnectPeer(addr string) {
	pm.connManager.Connect(addr)
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

func (pm *PeerManager) DisconnectPeer(peer *Peer) {
	if peer == nil {
		return
	}
	log.Trace("PeerManager disconnect peer:", peer.String())
	peer, ok := pm.RemovePeer(peer.ID())
	if ok {
		addr := peer.Addr().String()
		peer.Disconnect()
		pm.connManager.removeAddrFromConnectingList(addr)
		pm.addrManager.DisconnectedAddr(addr)
	}
}

func (pm *PeerManager) OnDiscardAddr(addr string) {
	pm.addrManager.DiscardAddr(addr)
}

func (pm *PeerManager) RandAddrs() []Addr {
	peers := pm.ConnectedPeers()

	log.Info("Rand peer addrs, connected peers:", peers)
	count := len(peers)
	if count > MaxOutboundCount {
		count = MaxOutboundCount
	}

	addrs := make([]Addr, count)
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

		peer := NewPeer(conn)
		go peer.Read()
	}
}

func (pm *PeerManager) makeMessage(cmd string) (Message, error) {
	var msg Message
	switch cmd {
	case "version":
		msg = new(Version)
	case "verack":
		msg = new(VerAck)
	case "getaddr":
		msg = new(AddrsReq)
	case "addr":
		msg = new(Addrs)
	default:
		return pm.msgHandler.MakeMessage(cmd)
	}

	return msg, nil
}

func (pm *PeerManager) handleMessage(peer *Peer, msg Message) {
	var err error
	switch msg := msg.(type) {
	case *Version:
		err = pm.OnVersion(peer, msg)
	case *VerAck:
		err = pm.OnVerAck(peer, msg)
	case *AddrsReq:
		err = pm.OnAddrsReq(peer, msg)
	case *Addrs:
		err = pm.OnAddrs(peer, msg)
	default:
		err = pm.msgHandler.HandleMessage(peer, msg)
	}

	if err != nil {
		log.Error("Handle message error,", err)
	}
}

func (pm *PeerManager) OnVersion(peer *Peer, v *Version) error {
	// Check if handshake with itself
	if v.Nonce == pm.Local().ID() {
		log.Error("SPV disconnect peer, peer handshake with itself")
		pm.DisconnectPeer(peer)
		pm.OnDiscardAddr(peer.Addr().String())
		return errors.New("Peer handshake with itself")
	}

	if peer.State() != INIT && peer.State() != HAND {
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
	if err := pm.msgHandler.OnHandshake(v); err != nil {
		pm.DisconnectPeer(peer)
		return err
	}

	var message Message
	if peer.State() == INIT {
		peer.SetState(HANDSHAKE)
		message = peer.NewVersionMsg()
	} else if peer.State() == HAND {
		peer.SetState(HANDSHAKED)
		message = new(VerAck)
	}

	go peer.Send(message)

	return nil
}

func (pm *PeerManager) OnVerAck(peer *Peer, va *VerAck) error {
	if peer.State() != HANDSHAKE && peer.State() != HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == HANDSHAKE {
		go peer.Send(new(VerAck))
	}

	peer.SetState(ESTABLISH)

	// Add to connected peer
	pm.AddConnectedPeer(peer)

	// Notify peer connected
	pm.msgHandler.OnPeerEstablish(peer)

	if pm.NeedMorePeers() {
		go peer.Send(new(AddrsReq))
	}

	return nil
}

func (pm *PeerManager) OnAddrs(peer *Peer, addrs *Addrs) error {
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

func (pm *PeerManager) OnAddrsReq(peer *Peer, req *AddrsReq) error {
	addrs := pm.RandAddrs()
	go peer.Send(NewAddrs(addrs))
	return nil
}
