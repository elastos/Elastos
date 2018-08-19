package net

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// needAddressThreshold is the number of addresses under which the
	// address manager will claim to need more addresses.
	needAddressThreshold = 100
	// addressListMonitorDuration is the time gap to check and remove
	// any bad addresses from address list
	addressListMonitorDuration = time.Hour * 12
)

type AddrManager struct {
	mutex       sync.RWMutex
	minOutbound int
	addresses   *addrList
	connected   *addrList
}

type addrList struct {
	list map[string]*knownAddress
}

func newAddressesList() *addrList {
	return &addrList{
		list: make(map[string]*knownAddress),
	}
}

func (a *addrList) put(key string, value *knownAddress) {
	a.list[key] = value
}

func (a *addrList) get(key string) *knownAddress {
	return a.list[key]
}

func (a *addrList) exist(key string) bool {
	_, ok := a.list[key]
	return ok
}

func (a *addrList) del(key string) {
	delete(a.list, key)
}

func (a *addrList) size() int {
	return len(a.list)
}

func newAddrManager(minOutbound int) *AddrManager {
	am := new(AddrManager)
	am.minOutbound = minOutbound
	am.addresses = newAddressesList()
	am.connected = newAddressesList()
	return am
}

func (am *AddrManager) NeedMoreAddresses() bool {
	am.mutex.RLock()
	defer am.mutex.RUnlock()
	return am.addresses.size() < needAddressThreshold
}

func (am *AddrManager) GetOutboundAddresses() []p2p.NetAddress {
	am.mutex.RLock()
	defer am.mutex.RUnlock()

	var addrs []p2p.NetAddress
	for _, addr := range SortAddressMap(am.addresses.list) {
		address := addr.String()
		// Skip connected address
		if am.connected.exist(address) {
			continue
		}
		addr.increaseAttempts()
		addr.updateLastAttempt()
		// Skip bad address
		if addr.isBad() {
			continue
		}
		addrs = append(addrs, addr.NetAddress)
		if len(addrs) >= am.minOutbound {
			break
		}
	}
	return addrs
}

func (am *AddrManager) RandGetAddresses() []p2p.NetAddress {
	am.mutex.RLock()
	defer am.mutex.RUnlock()

	var addrs []p2p.NetAddress
	for _, addr := range am.addresses.list {
		if addr.isBad() {
			continue
		}
		addrs = append(addrs, addr.NetAddress)
		if len(addrs) >= am.minOutbound*2 {
			break
		}
	}
	return addrs
}

func (am *AddrManager) AddressConnected(na *p2p.NetAddress) {
	am.mutex.Lock()
	defer am.mutex.Unlock()

	addr := na.String()
	// Try add to address list
	am.addOrUpdateAddress(na)
	if !am.connected.exist(addr) {
		ka := am.addresses.get(addr)
		ka.SaveAddr(na)
		am.connected.put(addr, ka)
	}
}

func (am *AddrManager) AddressDisconnect(na *p2p.NetAddress) {
	am.mutex.Lock()
	defer am.mutex.Unlock()

	addr := na.String()
	// Update disconnect time
	ka := am.addresses.get(addr)
	ka.updateLastDisconnect()
	// Delete from connected list
	am.connected.del(addr)
}

func (am *AddrManager) AddOrUpdateAddress(na *p2p.NetAddress) {
	am.mutex.Lock()
	defer am.mutex.Unlock()

	am.addOrUpdateAddress(na)
}

func (am *AddrManager) addOrUpdateAddress(na *p2p.NetAddress) {
	addr := na.String()
	// Update already known address
	ka := am.addresses.get(addr)
	if ka == nil {
		ka := new(knownAddress)
		ka.SaveAddr(na)
		// Add to address list
		am.addresses.put(addr, ka)
	} else {
		ka.SaveAddr(na)
	}
}

func (am *AddrManager) KnowAddresses() []p2p.NetAddress {
	am.mutex.RLock()
	defer am.mutex.RUnlock()

	nas := make([]p2p.NetAddress, 0, am.addresses.size())
	for _, ka := range am.addresses.list {
		nas = append(nas, ka.NetAddress)
	}
	return nas
}

func (am *AddrManager) monitorAddresses() {
	ticker := time.NewTicker(addressListMonitorDuration)
	defer ticker.Stop()

	for range ticker.C {
		am.mutex.Lock()
		for addr, ka := range am.addresses.list {
			if ka.isBad() {
				delete(am.addresses.list, addr)
			}
		}
		am.mutex.Unlock()
	}
}
