package net

import (
	"net"
	"sync"
	"time"

	"fmt"
	"github.com/elastos/Elastos.ELA.SPV/log"
)

const (
	DialTimeout      = time.Second * 10
	HandshakeTimeout = time.Second * 10
)

type ConnectionListener interface {
}

type ConnManager struct {
	port           uint16
	maxConnections int

	mutex       *sync.RWMutex
	connections map[string]net.Conn

	OnConnection func(conn net.Conn, inbound bool)
}

func newConnManager(port uint16, maxConnections int) *ConnManager {
	cm := new(ConnManager)
	cm.port = port
	cm.maxConnections = maxConnections
	cm.mutex = new(sync.RWMutex)
	cm.connections = make(map[string]net.Conn)
	return cm
}

func (cm *ConnManager) resolveAddr(addr string) (string, error) {
	tcpAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		log.Debugf("Can not resolve address %s", addr)
		return addr, err
	}
	return tcpAddr.String(), nil
}

func (cm *ConnManager) Connect(addr string) {
	tcpAddr, err := cm.resolveAddr(addr)
	if err != nil {
		return
	}

	if cm.IsConnected(tcpAddr) {
		log.Debugf("Seed %s already connected", addr)
		return
	}

	conn, err := net.DialTimeout("tcp", tcpAddr, DialTimeout)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		return
	}

	// Callback outbound connection
	cm.OnConnection(conn, false)
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
	listener, err := net.Listen("tcp", fmt.Sprint(":", cm.port))
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
		log.Debugf("New connection accepted, remote: %s local: %s", conn.RemoteAddr(), conn.LocalAddr())

		// Callback inbound connection
		cm.OnConnection(conn, true)
	}
}

func (cm *ConnManager) monitorConnections() {
	ticker := time.NewTicker(InfoUpdateDuration)
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
