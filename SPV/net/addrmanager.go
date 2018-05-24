package net

import (
	"fmt"
	"io/ioutil"
	"os"
	"sync"

	"encoding/json"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"time"
)

const (
	AddrLocalCache = "addrs.cache"
	// needAddressThreshold is the number of addresses under which the
	// address manager will claim to need more addresses.
	needAddressThreshold = 1000
	// addressListMonitorDuration is the time gap to check and remove
	// any bad addresses from address list
	addressListMonitorDuration = time.Hour * 12
)

type AddrManager struct {
	sync.RWMutex
	minOutbound int
	addrList    map[string]*knownAddress
	connected   map[string]*knownAddress
}

func newAddrManager(seeds []string, minOutbound int) *AddrManager {
	am := new(AddrManager)
	am.minOutbound = minOutbound
	am.addrList = make(map[string]*knownAddress)
	am.connected = make(map[string]*knownAddress)

	var addrList []string
	// Read cached addresses from file
	data, err := ioutil.ReadFile(AddrLocalCache)
	if err == nil {
		json.Unmarshal(data, &addrList)
	}
	// Add seeds to address list
	addrList = append(addrList, seeds...)
	for _, addr := range addrList {
		// Only add valid address
		if ka := NewKnownAddress(addr); ka != nil {
			am.addrList[addr] = ka
		}
	}

	return am
}

func (am *AddrManager) NeedMoreAddresses() bool {
	return len(am.addrList) < needAddressThreshold
}

func (am *AddrManager) GetOutboundAddresses(cm *ConnManager) []p2p.NetAddress {
	am.Lock()
	defer am.Unlock()

	var addrs []p2p.NetAddress
	addrListByChance := SortAddressMap(am.addrList)
	for _, addr := range addrListByChance {
		address := addr.String()
		// Skip connecting address
		if cm.isConnecting(address) {
			continue
		}
		// Skip connected address
		if _, ok := am.connected[address]; ok {
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
	am.Lock()
	defer am.Unlock()

	var addrs []p2p.NetAddress
	for _, addr := range am.addrList {
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
	am.Lock()
	defer am.Unlock()

	addr := na.String()
	// Try add to address list
	am.addOrUpdateAddress(na)
	if _, ok := am.connected[addr]; !ok {
		ka := am.addrList[addr]
		ka.SaveAddr(na)
		am.connected[addr] = ka
	}
}

func (am *AddrManager) AddressDisconnect(na *p2p.NetAddress) {
	am.Lock()
	defer am.Unlock()

	addr := na.String()
	// Update disconnect time
	ka := am.addrList[addr]
	ka.updateLastDisconnect()
	// Delete from connected list
	delete(am.connected, addr)
}

func (am *AddrManager) AddOrUpdateAddress(na *p2p.NetAddress) {
	am.Lock()
	defer am.Unlock()

	am.addOrUpdateAddress(na)
}

func (am *AddrManager) addOrUpdateAddress(na *p2p.NetAddress) {
	addr := na.String()
	// Update already known address
	if ka, ok := am.addrList[addr]; ok {
		ka.SaveAddr(na)
		return
	}
	ka := new(knownAddress)
	ka.SaveAddr(na)
	// Add to address list
	am.addrList[addr] = ka
	// Save to local
	am.saveToLocal()
}

func (am *AddrManager) saveToLocal() {
	var addrList []string
	for _, addr := range am.addrList {
		addrList = append(addrList, addr.String())
	}

	data, err := json.Marshal(addrList)
	if err != nil {
		return
	}

	file, err := os.OpenFile(AddrLocalCache, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, 0666)
	if err != nil {
		fmt.Println("Open cached addresses failed")
		return
	}

	_, err = file.Write(data)
	if err != nil {
		fmt.Println("Write cached addresses failed")
		return
	}
}

func (am *AddrManager) monitorAddresses() {
	ticker := time.NewTicker(addressListMonitorDuration)
	defer ticker.Stop()

	for range ticker.C {
		am.Lock()
		for _, ka := range am.addrList {
			if ka.isBad() {
				delete(am.addrList, ka.String())
			}
		}
		am.Unlock()
		am.saveToLocal()
	}
}
