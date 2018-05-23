package net

import (
	"net"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
)

const (
	ConnTimeOut   = 5
	RetryDuration = 15
	MaxRetryCount = 5
)

type ConnManager struct {
	sync.Mutex
	maxConnections  int
	connList        []string
	retryList       map[string]int
	connections     map[string]net.Conn
	onPeerConnected func(conn net.Conn)
}

func newConnManager(maxConnections int, onPeerConnected func(conn net.Conn)) *ConnManager {
	cm := new(ConnManager)
	cm.retryList = make(map[string]int)
	cm.connections = make(map[string]net.Conn)
	cm.maxConnections = maxConnections
	cm.onPeerConnected = onPeerConnected
	return cm
}

func (cm *ConnManager) Connect(addr string) {
	cm.Lock()
	defer cm.Unlock()

	if cm.isConnecting(addr) {
		log.Infof("ConnManager %s is connecting,", addr)
		return
	}

	cm.connList = append(cm.connList, addr)
	go cm.connectPeer(addr)
}

func (cm *ConnManager) isConnecting(addr string) bool {
	for _, connAddr := range cm.connList {
		if connAddr == addr {
			return true
		}
	}
	return false
}

func (cm *ConnManager) disConnecting(addr string) {
	delete(cm.retryList, addr)
	for i, connAddr := range cm.connList {
		if connAddr == addr {
			cm.connList = append(cm.connList[:i], cm.connList[i+1:]...)
			return
		}
	}
}

func (cm *ConnManager) Disconnected(addr string) {
	cm.Lock()
	defer cm.Unlock()

	cm.disConnecting(addr)
	conn := cm.connections[addr]
	conn.Close()
	delete(cm.connections, addr)
}

func (cm *ConnManager) connectPeer(addr string) {
	conn, err := net.DialTimeout("tcp", addr, time.Second*ConnTimeOut)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		cm.retry(addr)
		return
	}

	// Dis connecting address
	cm.disConnecting(addr)
	// Add to connection list
	cm.connections[addr] = conn
	// Callback connection
	cm.onPeerConnected(conn)
}

func (cm *ConnManager) retry(addr string) {
	cm.Lock()
	retryTimes, ok := cm.retryList[addr]
	if !ok {
		retryTimes = 0
	} else {
		retryTimes += 1
	}
	log.Info("Put into retry queue, retry times:", retryTimes)
	if retryTimes > MaxRetryCount {
		cm.disConnecting(addr)
		cm.Unlock()
		return
	}
	cm.retryList[addr] = retryTimes
	cm.Unlock()

	log.Info("Wait for retry ", addr)
	time.Sleep(time.Second * RetryDuration)
	cm.connectPeer(addr)
}

func (cm *ConnManager) monitorConnections() {
	ticker := time.NewTicker(InfoUpdateDuration)
	for range ticker.C {
		cm.Lock()
		for len(cm.connections) > cm.maxConnections {
			// Random close a connection
			for addr, conn := range cm.connections {
				conn.Close()
				delete(cm.connections, addr)
				break // back to connections count check
			}
		}
		cm.Unlock()
	}
}
