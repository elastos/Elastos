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

type connMsg struct {
	inbound  bool
	conn     net.Conn
}

type ConnectionListener interface {
	OnConnection(msg connMsg)
}

type ConnManager struct {
	localPeer      *Peer
	maxConnections int

	mutex       *sync.RWMutex
	connections map[string]net.Conn

	listener ConnectionListener
}

func newConnManager(localPeer *Peer, maxConnections int, listener ConnectionListener) *ConnManager {
	cm := new(ConnManager)
	cm.localPeer = localPeer
	cm.maxConnections = maxConnections
	cm.mutex = new(sync.RWMutex)
	cm.connections = make(map[string]net.Conn)
	cm.listener = listener
	return cm
}

func (cm *ConnManager) ResolveAddr(addr string) (string, error) {
	tcpAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		log.Debugf("Can not resolve address %s", addr)
		return addr, err
	}

	log.Debugf("Seed %s, resolved addr %s", addr, tcpAddr.String())
	return tcpAddr.String(), nil
}

func (cm *ConnManager) Connect(addr string) {
	log.Debugf("Connect addr %s", addr)

	conn, err := net.DialTimeout("tcp", addr, time.Second*ConnTimeOut)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		return
	}

	// Callback outbound connection
	cm.listener.OnConnection(connMsg{inbound: false, conn: conn})
}

func (cm *ConnManager) IsConnected(addr string) bool {
	cm.mutex.RLock()
	defer cm.mutex.RUnlock()
	_, ok := cm.connections[addr]
	return ok
}

func (cm *ConnManager) PeerConnected(addr string, conn net.Conn) {
	cm.mutex.Lock()
	defer cm.mutex.Unlock()
	// Add to connection list
	cm.connections[addr] = conn
}

func (cm *ConnManager) PeerDisconnected(addr string) {
	cm.mutex.Lock()
	defer cm.mutex.Unlock()

	if _, ok := cm.connections[addr]; ok {
		delete(cm.connections, addr)
	}
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

		// Callback inbound connection
		cm.listener.OnConnection(connMsg{inbound: true, conn: conn})
	}
}

func (cm *ConnManager) monitorConnections() {
	ticker := time.NewTicker(time.Second * InfoUpdateDuration)
	for range ticker.C {
		cm.mutex.Lock()
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
		cm.mutex.Unlock()
	}
}
