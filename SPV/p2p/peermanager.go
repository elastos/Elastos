package p2p

import (
	"errors"
	"fmt"
	"os"
	"time"
	"net"

	"SPVWallet/core/transaction"
	"SPVWallet/db"
)

const (
	MinConnCount       = 4
	InfoUpdateDuration = 5
	KeepAliveTimeout   = 3
	MaxOutboundCount   = 6
)

var instance *PeerManager

type PeerManager struct {
	*Peers
	*SyncManager
	addrManager *AddrManager
	connManager *ConnManager
}

func GetPeerManager() *PeerManager {
	if instance == nil {
		fmt.Errorf("Peer manager was not initialized, please run Init(localPeer *Peer) frist")
		os.Exit(0)
	}

	return instance
}

func (pm *PeerManager) Start() {
	pm.connectPeers()
	go pm.listenConnection()
	go pm.keepUpdate()
}

func (pm *PeerManager) needMorePeers() bool {
	return pm.PeersCount() < MinConnCount
}

func (pm *PeerManager) DisconnectPeer(peer *Peer) {
	peer, ok := pm.RemovePeer(peer.ID())
	if ok {
		peer.Disconnect()
		pm.addrManager.DisconnectedAddr(peer.Addr().TCPAddr())
	}
}

func (pm *PeerManager) OnDiscardAddr(addr string) {
	pm.addrManager.DiscardAddr(addr)
}

func (pm *PeerManager) RandPeerAddrs() []PeerAddr {
	peers := pm.ConnectedPeers()

	count := len(peers)
	if count > MaxOutboundCount {
		count = MaxOutboundCount
	}

	addrs := make([]PeerAddr, count)
	for count > 0 {
		count--
		addrs = append(addrs, *peers[count].Addr())
	}

	return addrs
}

func (pm *PeerManager) connectPeers() {
	if pm.needMorePeers() {
		addrs := pm.addrManager.GetIdleAddrs(MaxOutboundCount)
		for _, addr := range addrs {
			go pm.connManager.connectPeer(addr)
		}
	}
}

func (pm *PeerManager) listenConnection() {
	listener, err := net.Listen("tcp", fmt.Sprint(":", pm.Local().port))
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

func (pm *PeerManager) keepUpdate() {
	ticker := time.NewTicker(time.Second * InfoUpdateDuration)
	defer ticker.Stop()
	for range ticker.C {

		// Update peers info
		for _, peer := range pm.ConnectedPeers() {
			if peer.State() == ESTABLISH {

				// Disconnect inactive peer
				if peer.LastActive().Before(
					time.Now().Add(-time.Second * InfoUpdateDuration * KeepAliveTimeout)) {
					pm.DisconnectPeer(peer)
					continue
				}

				// Send ping message to peer
				msg, err := NewPingMsg()
				if err != nil {
					fmt.Println("Failed to build ping message, ", err)
					return
				}
				go peer.Send(msg)
			}
		}

		// Keep connections
		pm.connectPeers()

		// Keep synchronizing blocks
		pm.SyncBlocks()
	}
}

func (pm *PeerManager) OnVersion(peer *Peer, v *Version) error {
	// Check if handshake with itself
	if v.Nonce == pm.Local().ID() {
		peer.Disconnect()
		return errors.New("Peer handshake with itself")
	}

	if v.Version < PeerVersion {
		peer.Disconnect()
		return errors.New(fmt.Sprint("To support SPV protocol, peer version must greater than ", PeerVersion))
	}

	if v.Services/ServiceSPV&1 == 0 {
		peer.Disconnect()
		return errors.New("SPV service not enabled on connected peer")
	}

	if peer.State() != INIT && peer.State() != HAND {
		return errors.New("Unknow status to received version")
	}

	// Remove duplicate peer connection
	knownPeer, ok := pm.RemovePeer(v.Nonce)
	if ok {
		fmt.Println("Reconnect peer ", v.Nonce)
		knownPeer.Disconnect()
	}

	// Update peer info with version message
	peer.Update(v)

	// Add peer to list
	pm.addConnectedPeer(peer)

	// Mark addr as connected
	pm.addrManager.addConnectedAddr(peer.Addr().TCPAddr())

	var msg []byte
	if peer.State() == INIT {
		peer.SetState(HANDSHAKE)
		msg, _ = NewVersionMsg(peer)
	} else if peer.State() == HAND {
		peer.SetState(HANDSHAKED)
		msg, _ = NewVerAckMsg()
	}

	go peer.Send(msg)

	return nil
}

func (pm *PeerManager) OnVerAck(peer *Peer, va *VerAck) error {
	if peer.State() != HANDSHAKE && peer.State() != HANDSHAKED {
		return errors.New("Unknow status to received verack")
	}

	if peer.State() == HANDSHAKE {
		msg, _ := NewVerAckMsg()
		go peer.Send(msg)
	}

	peer.SetState(ESTABLISH)

	// Remove from connecting list
	pm.connManager.removeAddrFromConnectingList(peer.Addr().TCPAddr())

	msg, _ := NewFilterLoadMsg(db.GetBlockchain().GetFilter())
	go peer.Send(msg)

	if pm.needMorePeers() {
		msg, _ := NewAddrsReqMsg()
		go peer.Send(msg)
	}

	return nil
}

func (pm *PeerManager) OnPing(peer *Peer, p *Ping) error {
	peer.SetHeight(p.Height)

	msg, err := NewPongMsg()
	if err != nil {
		fmt.Println("Failed to build pong message")
		return err
	}

	go peer.Send(msg)

	return nil
}

func (pm *PeerManager) OnPong(peer *Peer, p *Pong) error {
	peer.SetHeight(p.Height)
	return nil
}

func (pm *PeerManager) OnAddrs(peer *Peer, addrs *Addrs) error {
	for _, addr := range addrs.PeerAddrs {
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
		if pm.needMorePeers() {
			pm.connManager.Connect(addr.TCPAddr())
		}
	}

	return nil
}

func (pm *PeerManager) OnAddrsReq(peer *Peer, req *AddrsReq) error {
	addrs := pm.RandPeerAddrs()
	msg, err := NewAddrsMsg(addrs)
	if err != nil {
		return err
	}

	go peer.Send(msg)

	return nil
}

func (pm *PeerManager) OnInventory(peer *Peer, inv *Inventory) error {
	switch inv.Type {
	case TRANSACTION:
		// Do nothing, transaction inventory is not supported
	case BLOCK:
		return pm.HandleBlockInvMsg(inv, peer)
	}
	return nil
}

func (pm *PeerManager) SendTransaction(txn *transaction.Transaction) error {
	txnMsg, err := NewTxnMsg(*txn)
	if err != nil {
		return err
	}

	// Broadcast transaction to connected peers
	pm.Broadcast(txnMsg)

	return nil
}
