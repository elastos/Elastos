package p2p

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

	connList  []string
	retryList map[string]int

	OnDiscardAddr func(add string)
}

func newConnManager(onDiscardAddr func(add string)) *ConnManager {
	cm := new(ConnManager)
	cm.retryList = make(map[string]int)
	cm.OnDiscardAddr = onDiscardAddr
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

	// Start read msg from remote peer
	remote := NewPeer(conn)
	remote.SetState(HAND)
	go remote.Read()

	// Send version message to remote peer
	go remote.Send(NewVersion())
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
		cm.OnDiscardAddr(addr)
		return
	}
	cm.retryList[addr] = retryTimes
	cm.Unlock()

	log.Info("Wait for retry ", addr)
	time.Sleep(time.Second * RetryDuration)
	cm.connectPeer(addr)
}
