package p2p

import (
	"fmt"
	"net"

	"SPVWallet/p2p/msg"
	"SPVWallet/log"
)

const (
	MinConnCount       = 4
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 3
	MaxOutboundCount   = 6
)

type PeerManager struct {
	*Peers
	addrManager *AddrManager
	connManager *ConnManager
}

func NewPeerManager(clientId uint64, localPort uint16, seeds []string) *PeerManager {
	// Initiate PeerManager
	pm := new(PeerManager)
	pm.Peers = newPeers(clientId, localPort)
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

	addr := peer.Addr().TCPAddr()

	// Remove addr from connecting list
	pm.connManager.removeAddrFromConnectingList(addr)

	// Mark addr as connected
	pm.addrManager.AddAddr(addr)

	// Listen peer disconnect
	peer.OnDisconnect = pm.DisconnectPeer
}

func (pm *PeerManager) RemoveFromConnectingList(peer *Peer) {

}

func (pm *PeerManager) DisconnectPeer(peer *Peer) {
	log.Trace("PeerManager disconnect peer:", peer.String())
	peer, ok := pm.RemovePeer(peer.ID())
	if ok {
		addr := peer.Addr().TCPAddr()
		peer.Disconnect()
		pm.connManager.removeAddrFromConnectingList(addr)
		pm.addrManager.DisconnectedAddr(addr)
	}
}

func (pm *PeerManager) OnDiscardAddr(addr string) {
	pm.addrManager.DiscardAddr(addr)
}

func (pm *PeerManager) RandPeerAddrs() []msg.PeerAddr {
	peers := pm.ConnectedPeers()

	log.Info("Rand peer addrs, connected peers:", peers)
	count := len(peers)
	if count > MaxOutboundCount {
		count = MaxOutboundCount
	}

	addrs := make([]msg.PeerAddr, count)
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
