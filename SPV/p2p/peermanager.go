package p2p

import (
	"errors"
	"fmt"
	"net"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
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

	OnMakeMessage   func(cmd string) (Message, error)
	OnHandleVersion func(v *Version) error
	OnPeerConnected func(peer *Peer)
	OnHandleMessage func(peer *Peer, msg Message) error
}

func InitPeerManager(localPeer *Peer, seeds []string) *PeerManager {
	// Initiate PeerManager
	pm = new(PeerManager)
	pm.Peers = newPeers(localPeer)
	pm.addrManager = newAddrManager(seeds)
	pm.connManager = newConnManager(pm.OnDiscardAddr)
	return pm
}

func (pm *PeerManager) Start() {
	log.Info("PeerManager start")
	pm.ConnectPeers()
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

	// Listen peer disconnect
	peer.OnDisconnect = pm.DisconnectPeer
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

func (pm *PeerManager) ConnectPeers() {
	if pm.NeedMorePeers() {
		addrs := pm.addrManager.GetIdleAddrs(MaxOutboundCount)
		for _, addr := range addrs {
			go pm.ConnectPeer(addr)
		}
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

func (pm *PeerManager) OnVersion(peer *Peer, v *Version) error {
	log.Info("SPV OnVersion")
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

	// Handle version message
	if err := pm.OnHandleVersion(v); err != nil {
		pm.DisconnectPeer(peer)
		return err
	}

	var message Message
	if peer.State() == INIT {
		peer.SetState(HANDSHAKE)
		message = NewVersion()
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
	pm.OnPeerConnected(peer)

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
