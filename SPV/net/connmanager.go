package net

import (
	"net"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"fmt"
)

const (
	ConnTimeOut   = 5
	RetryDuration = 15
	MaxRetryCount = 5
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

	retryLock *sync.RWMutex
	retryList map[string]int

	connsLock   *sync.RWMutex
	connections map[string]net.Conn

	listener ConnectionListener
}

func newConnManager(localPeer *Peer, maxConnections int, listener ConnectionListener) *ConnManager {
	cm := new(ConnManager)
	cm.localPeer = localPeer
	cm.maxConnections = maxConnections
	cm.connectingLock = new(sync.RWMutex)
	cm.connectingList = make(map[string]string)
	cm.retryLock = new(sync.RWMutex)
	cm.retryList = make(map[string]int)
	cm.connsLock = new(sync.RWMutex)
	cm.connections = make(map[string]net.Conn)
	cm.listener = listener
	return cm
}

func (cm *ConnManager) Connect(addr string) {
	if cm.isConnecting(addr) {
		return
	}

	cm.connectingLock.Lock()
	cm.connectingList[addr] = addr
	cm.connectingLock.Unlock()

	go cm.connectPeer(addr)
}

func (cm *ConnManager) isConnecting(addr string) bool {
	cm.connectingLock.RLock()
	_, ok := cm.connectingList[addr]
	cm.connectingLock.RUnlock()
	return ok
}

func (cm *ConnManager) deConnecting(addr string) {
	cm.retryLock.Lock()
	delete(cm.retryList, addr)
	cm.retryLock.Unlock()

	cm.connectingLock.Lock()
	delete(cm.connectingList, addr)
	cm.connectingLock.Unlock()
}

func (cm *ConnManager) isConnected(addr string) bool {
	cm.connsLock.RLock()
	_, ok := cm.connections[addr]
	cm.connsLock.RUnlock()
	return ok
}

func (cm *ConnManager) PeerDisconnected(addr string) {
	cm.deConnecting(addr)

	cm.connsLock.Lock()
	if conn, ok := cm.connections[addr]; ok {
		conn.Close()
		delete(cm.connections, addr)
	}
	cm.connsLock.Unlock()
}

func (cm *ConnManager) connectPeer(addr string) {
	conn, err := net.DialTimeout("tcp", addr, time.Second*ConnTimeOut)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		cm.retry(addr)
		return
	}

	// de connecting address
	cm.deConnecting(addr)
	// Callback connection
	cm.listener.OnOutbound(conn)
}

func (cm *ConnManager) PeerConnected(addr string, conn net.Conn) {
	// Add to connection list
	cm.connsLock.Lock()
	cm.connections[addr] = conn
	cm.connsLock.Unlock()
}

func (cm *ConnManager) retry(addr string) {
	cm.retryLock.RLock()
	retryTimes, ok := cm.retryList[addr]
	if !ok {
		retryTimes = 0
	} else {
		retryTimes += 1
	}
	cm.retryLock.RUnlock()
	log.Info("Put into retry queue, retry times:", retryTimes)
	if retryTimes > MaxRetryCount {
		cm.deConnecting(addr)
		return
	}
	cm.retryLock.Lock()
	cm.retryList[addr] = retryTimes
	cm.retryLock.Unlock()

	log.Info("Wait for retry ", addr)
	time.Sleep(time.Second * RetryDuration)
	cm.connectPeer(addr)
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
		fmt.Printf("New connection accepted, remote: %s local: %s\n", conn.RemoteAddr(), conn.LocalAddr())

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
