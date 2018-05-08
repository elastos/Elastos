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

type ConnectHandler interface {
	OnPeerConnected(conn net.Conn)
	OnDiscardAddr(addr string)
}

type ConnManager struct {
	sync.Mutex

	connList  []string
	retryList map[string]int

	handler ConnectHandler
}

func newConnManager(handler ConnectHandler) *ConnManager {
	cm := new(ConnManager)
	cm.retryList = make(map[string]int)
	cm.handler = handler
	return cm
}

func (cm *ConnManager) Connect(addr string) {
	cm.Lock()
	defer cm.Unlock()

	if cm.inConnList(addr) {
		log.Info("ConnManager addr in connection list,", addr)
		return
	}

	cm.connList = append(cm.connList, addr)
	go cm.connectPeer(addr)
}

func (cm *ConnManager) inConnList(addr string) bool {
	for _, connAddr := range cm.connList {
		if connAddr == addr {
			return true
		}
	}
	return false
}

func (cm *ConnManager) removeAddrFromConnectingList(addr string) {
	delete(cm.retryList, addr)
	for i, connAddr := range cm.connList {
		if connAddr == addr {
			cm.connList = append(cm.connList[:i], cm.connList[i+1:]...)
			return
		}
	}
}

func (cm *ConnManager) connectPeer(addr string) {
	conn, err := net.DialTimeout("tcp", addr, time.Second*ConnTimeOut)
	if err != nil {
		log.Error("Connect to addr ", addr, " failed, err", err)
		cm.retry(addr)
		return
	}

	cm.handler.OnPeerConnected(conn)
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
		cm.removeAddrFromConnectingList(addr)
		cm.Unlock()
		// Discard useless address
		cm.handler.OnDiscardAddr(addr)
		return
	}
	cm.retryList[addr] = retryTimes
	cm.Unlock()

	log.Info("Wait for retry ", addr)
	time.Sleep(time.Second * RetryDuration)
	cm.connectPeer(addr)
}
