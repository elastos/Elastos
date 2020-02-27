// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package addrmgr

import (
	"encoding/base32"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

// AddrManager provides a concurrency safe address manager for caching potential
// peers on the network.
type AddrManager struct {
	mtx       sync.Mutex
	peersFile string
	addrIndex map[[33]byte]net.Addr // address key to ka for all addrs.
	started   int32
	shutdown  int32
	wg        sync.WaitGroup
	quit      chan struct{}
}

type serializedNetAddress struct {
	PID  [33]byte
	Addr string
}

type serializedAddrManager struct {
	Addresses []*serializedNetAddress
}

const (
	// dumpAddressInterval is the interval used to dump the address
	// cache to disk for future use.
	dumpAddressInterval = time.Minute * 10
)

// addressHandler is the main handler for the address manager.  It must be run
// as a goroutine.
func (a *AddrManager) addressHandler() {
	dumpAddressTicker := time.NewTicker(dumpAddressInterval)
	defer dumpAddressTicker.Stop()
out:
	for {
		select {
		case <-dumpAddressTicker.C:
			a.savePeers()

		case <-a.quit:
			break out
		}
	}
	a.savePeers()
	a.wg.Done()
}

// savePeers saves all the known addresses to a file so they can be read back
// in at next run.
func (a *AddrManager) savePeers() {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	w, err := os.OpenFile(a.peersFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0600)
	if err != nil {
		log.Errorf("Error opening file %s: %v", a.peersFile, err)
		return
	}
	defer w.Close()

	// First we make a serialisable datastructure so we can encode it to
	// json.
	sam := new(serializedAddrManager)
	sam.Addresses = make([]*serializedNetAddress, 0, len(a.addrIndex))
	for k, v := range a.addrIndex {
		ska := new(serializedNetAddress)
		copy(ska.PID[:], k[:])
		ska.Addr = v.String()
		// Tried and refs are implicit in the rest of the structure
		// and will be worked out from context on unserialisation.
		sam.Addresses = append(sam.Addresses, ska)
	}

	enc := json.NewEncoder(w)
	if err := enc.Encode(&sam); err != nil {
		log.Errorf("Failed to encode file %s: %v", a.peersFile, err)
		return
	}
}

// loadPeers loads the known address from the saved file.  If empty, missing, or
// malformed file, just don't load anything and start fresh
func (a *AddrManager) loadPeers() {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	err := a.deserializePeers(a.peersFile)
	if err != nil {
		log.Errorf("Failed to parse file %s: %v", a.peersFile, err)
		// if it is invalid we nuke the old one unconditionally.
		err = os.Remove(a.peersFile)
		if err != nil {
			log.Warnf("Failed to remove corrupt peers file %s: %v",
				a.peersFile, err)
		}
		return
	}
	log.Infof("Loaded %d addresses from file '%s'", len(a.addrIndex), a.peersFile)
}

func (a *AddrManager) deserializePeers(filePath string) error {
	_, err := os.Stat(filePath)
	if os.IsNotExist(err) {
		return nil
	}
	r, err := os.OpenFile(filePath, os.O_RDONLY, 0400)
	if err != nil {
		return fmt.Errorf("%s error opening file: %v", filePath, err)
	}
	defer r.Close()

	var sam serializedAddrManager
	dec := json.NewDecoder(r)
	err = dec.Decode(&sam)
	if err != nil {
		return fmt.Errorf("error reading %s: %v", filePath, err)
	}

	for _, v := range sam.Addresses {
		na, err := a.DeserializeNetAddress(v.Addr)
		if err != nil {
			return fmt.Errorf("failed to deserialize netaddress "+
				"%s: %v", v.Addr, err)
		}
		a.addrIndex[v.PID] = na
	}

	return nil
}

// DeserializeNetAddress converts a given address string to a *p2p.NetAddress
func (a *AddrManager) DeserializeNetAddress(addr string) (net.Addr, error) {
	host, portStr, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, err
	}
	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}

	return a.HostToNetAddress(host, uint16(port), 0)
}

// Start begins the core address handler which manages a pool of known
// addresses, timeouts, and interval based writes.
func (a *AddrManager) Start() {
	// Already started?
	if atomic.AddInt32(&a.started, 1) != 1 {
		return
	}

	// Load peers we already know about from file.
	a.loadPeers()

	// Start the address ticker to save addresses periodically.
	a.wg.Add(1)
	go a.addressHandler()
}

// Stop gracefully shuts down the address manager by stopping the main handler.
func (a *AddrManager) Stop() {
	if atomic.AddInt32(&a.shutdown, 1) != 1 {
		log.Warnf("Address manager is already in the process of " +
			"shutting down")
		return
	}

	log.Infof("Address manager shutting down")
	close(a.quit)
	a.wg.Wait()
}

// AddAddress adds a new address to the address manager.  It enforces a max
// number of addresses and silently ignores duplicate addresses.  It is
// safe for concurrent access.
func (a *AddrManager) AddAddress(pid [33]byte, addr net.Addr) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	a.addrIndex[pid] = addr
}

// HostToNetAddress returns a netaddress given a host address.  If the address
// is a Tor .onion address this will be taken care of.  Else if the host is
// not an IP address it will be resolved (via Tor if required).
func (a *AddrManager) HostToNetAddress(host string, port uint16, services uint64) (net.Addr, error) {
	// Tor address is 16 char base32 + ".onion"
	var ip net.IP
	if len(host) == 22 && host[16:] == ".onion" {
		// go base32 encoding uses capitals (as does the rfc
		// but Tor and tend to user lowercase, so we switch
		// case here.
		data, err := base32.StdEncoding.DecodeString(
			strings.ToUpper(host[:16]))
		if err != nil {
			return nil, err
		}
		prefix := []byte{0xfd, 0x87, 0xd8, 0x7e, 0xeb, 0x43}
		ip = net.IP(append(prefix, data...))
	} else if ip = net.ParseIP(host); ip == nil {
		ips, err := net.LookupIP(host)
		if err != nil {
			return nil, err
		}
		if len(ips) == 0 {
			return nil, fmt.Errorf("no addresses found for %s", host)
		}
		ip = ips[0]
	}

	return &net.TCPAddr{IP: ip, Port: int(port)}, nil
}

// GetAddress returns the network address according to the given PID and Encode.
func (a *AddrManager) GetAddress(pid [33]byte) net.Addr {
	// Protect concurrent access.
	a.mtx.Lock()
	defer a.mtx.Unlock()

	return a.addrIndex[pid]
}

// New returns a new address manager.
// Use Start to begin processing asynchronous address updates.
func New(dataDir string) *AddrManager {
	am := AddrManager{
		peersFile: filepath.Join(dataDir, "peers.json"),
		addrIndex: make(map[[33]byte]net.Addr),
		quit:      make(chan struct{}),
	}
	return &am
}
