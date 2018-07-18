package net

import (
	"net"
	"sync"
	"time"

	"fmt"
	"github.com/elastos/Elastos.ELA.SPV/log"
)

const (
	ConnTimeOut      = 5
	HandshakeTimeout = 3
)

type ConnectionListener interface {
	OnOutbound(conn net.Conn)
	OnInbound(conn net.Conn)
}

type ConnManager struct {
	localPeer      *Peer
	maxConnections int

	connectingLock *sync.RWMutex
	connectingList map[string]string

	connsLock   *sync.RWMutex
	handshakes  map[net.Conn]struct{}
	connections map[string]net.Conn

	listener ConnectionListener
}

func newConnManager(localPeer *Peer, maxConnections int, listener ConnectionListener) *ConnManager {
	cm := new(ConnManager)
	cm.localPeer = localPeer
	cm.maxConnections = maxConnections
	cm.connectingLock = new(sync.RWMutex)
	cm.connectingList = make(map[string]string)
	cm.connsLock = new(sync.RWMutex)
	cm.handshakes = make(map[net.Conn]struct{})
	cm.connections = make(map[string]net.Conn)
	cm.listener = listener
	return cm
}

func (cm *ConnManager) Connect(addr string) {
	if cm.IsConnecting(addr) {
		return
	}

	cm.connectingLock.Lock()
	cm.connectingList[addr] = addr
	cm.connectingLock.Unlock()

	cm.connectPeer(addr)
}

func (cm *ConnManager) IsConnecting(addr string) bool {
	cm.connectingLock.RLock()
	defer cm.connectingLock.RUnlock()
	_, ok := cm.connectingList[addr]
	return ok
}

func (cm *ConnManager) DeConnecting(addr string) {
	cm.connectingLock.Lock()
	defer cm.connectingLock.Unlock()
	delete(cm.connectingList, addr)
}

func (cm *ConnManager) IsConnected(addr string) bool {
	cm.connsLock.RLock()
	defer cm.connsLock.RUnlock()
	_, ok := cm.connections[addr]
	return ok
}

func (cm *ConnManager) connectPeer(addr string) {
	cm.connsLock.RLock()
	if len(cm.handshakes)+len(cm.connections) > cm.maxConnections {
		cm.connsLock.RUnlock()
		return
	}
	cm.connsLock.RUnlock()

	conn, err := net.DialTimeout("tcp", addr, time.Second*ConnTimeOut)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		return
	}

	// Start handshake
	cm.StartHandshake(conn)
	// de connecting address
	cm.DeConnecting(addr)
	// Callback connection
	cm.listener.OnOutbound(conn)
}

func (cm *ConnManager) StartHandshake(conn net.Conn) {
	cm.connsLock.Lock()
	defer cm.connsLock.Unlock()
	// Put into handshakes
	cm.handshakes[conn] = struct{}{}

	// Close handshake timeout connections
	go cm.handleTimeout(conn)
}

func (cm *ConnManager) handleTimeout(conn net.Conn) {
	time.Sleep(time.Second * HandshakeTimeout)
	cm.connsLock.Lock()
	if _, ok := cm.handshakes[conn]; ok {
		conn.Close()
		delete(cm.handshakes, conn)
	}
	cm.connsLock.Unlock()
}

func (cm *ConnManager) PeerConnected(addr string, conn net.Conn) {
	cm.connsLock.Lock()
	defer cm.connsLock.Unlock()
	// Remove from handshakes
	delete(cm.handshakes, conn)
	// Add to connection list
	cm.connections[addr] = conn
}

func (cm *ConnManager) PeerDisconnected(addr string) {
	cm.DeConnecting(addr)

	cm.connsLock.Lock()
	if conn, ok := cm.connections[addr]; ok {
		conn.Close()
		delete(cm.connections, addr)
	}
	cm.connsLock.Unlock()
}

func (cm *ConnManager) listenConnection() {
	listener, err := net.Listen("tcp", fmt.Sprint(":", cm.localPeer.port))
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
		log.Debugf("New connection accepted, remote: %s local: %s\n", conn.RemoteAddr(), conn.LocalAddr())

		cm.StartHandshake(conn)
		cm.listener.OnInbound(conn)
	}
}

func (cm *ConnManager) monitorConnections() {
	ticker := time.NewTicker(time.Second * InfoUpdateDuration)
	for range ticker.C {
		cm.connsLock.Lock()
		conns := len(cm.connections)
		if conns > cm.maxConnections {
			// Random close connections
			for _, conn := range cm.connections {
				conn.Close()
				conns--
				if conns <= cm.maxConnections {
					break
				}
			}
		}
		cm.connsLock.Unlock()
	}
}
