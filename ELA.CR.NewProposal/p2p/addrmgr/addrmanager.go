// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package addrmgr

import (
	"container/list"
	crand "crypto/rand" // for seeding
	"encoding/base32"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"io"
	"math/rand"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

// AddrManager provides a concurrency safe address manager for caching potential
// peers on the network.
type AddrManager struct {
	mtx            sync.Mutex
	filter         p2p.NAFilter
	peersFile      string
	rand           *rand.Rand
	key            [32]byte
	addrIndex      map[string]*KnownAddress // address key to ka for all addrs.
	addrNew        [newBucketCount]map[string]*KnownAddress
	addrTried      [triedBucketCount]*list.List
	started        int32
	shutdown       int32
	wg             sync.WaitGroup
	quit           chan struct{}
	nTried         int
	nNew           int
	lamtx          sync.Mutex
	localAddresses map[string]*localAddress

	checkAddr func(addr string) error
}

type serializedKnownAddress struct {
	Addr        string
	Src         string
	Attempts    int
	TimeStamp   int64
	LastAttempt int64
	LastSuccess int64
	// no refcount or tried, that is available from context.
}

type serializedAddrManager struct {
	Version      int
	Key          [32]byte
	Addresses    []*serializedKnownAddress
	NewBuckets   [newBucketCount][]string // string is NetAddressKey
	TriedBuckets [triedBucketCount][]string
}

type localAddress struct {
	na    *p2p.NetAddress
	score AddressPriority
}

// AddressPriority type is used to describe the hierarchy of local address
// discovery methods.
type AddressPriority int

const (
	// InterfacePrio signifies the address is on a local interface
	InterfacePrio AddressPriority = iota

	// BoundPrio signifies the address has been explicitly bounded to.
	BoundPrio

	// UpnpPrio signifies the address was obtained from UPnP.
	UpnpPrio

	// HTTPPrio signifies the address was obtained from an external HTTP service.
	HTTPPrio

	// ManualPrio signifies the address was provided by --externalip.
	ManualPrio
)

const (
	// needAddressThreshold is the number of addresses under which the
	// address manager will claim to need more addresses.
	needAddressThreshold = 1000

	// dumpAddressInterval is the interval used to dump the address
	// cache to disk for future use.
	dumpAddressInterval = time.Minute * 10

	// expireAddressInterval is the interval used to check the address.
	expireAddressInterval = time.Hour * 2

	// triedBucketSize is the maximum number of addresses in each
	// tried address bucket.
	triedBucketSize = 256

	// triedBucketCount is the number of buckets we split tried
	// addresses over.
	triedBucketCount = 64

	// newBucketSize is the maximum number of addresses in each new address
	// bucket.
	newBucketSize = 64

	// newBucketCount is the number of buckets that we spread new addresses
	// over.
	newBucketCount = 1024

	// triedBucketsPerGroup is the number of tried buckets over which an
	// address group will be spread.
	triedBucketsPerGroup = 8

	// newBucketsPerGroup is the number of new buckets over which an
	// source address group will be spread.
	newBucketsPerGroup = 64

	// newBucketsPerAddress is the number of buckets a frequently seen new
	// address may end up in.
	newBucketsPerAddress = 8

	// numMissingDays is the number of days before which we assume an
	// address has vanished if we have not seen it announced  in that long.
	numMissingDays = 30

	// numRetries is the number of tried without a single success before
	// we assume an address is bad.
	numRetries = 3

	// maxFailures is the maximum number of failures we will accept without
	// a success before considering an address bad.
	maxFailures = 10

	// minBadDays is the number of days since the last success before we
	// will consider evicting an address.
	minBadDays = 7

	// getAddrMin is the minimum addresses that we will send in response
	// to a getAddr
	getAddrMin = 10

	// getAddrMax is the most addresses that we will send in response
	// to a getAddr (in practise the most addresses we will return from a
	// call to AddressCache()).
	getAddrMax = 2500

	// getAddrPercent is the percentage of total addresses known that we
	// will share with a call to AddressCache.
	getAddrPercent = 23

	// serialisationVersion is the current version of the on-disk format.
	serialisationVersion = 1
)

// SetCheckAddr used to set function to check address in addrManager.
func (a *AddrManager) SetCheckAddr(checkAddr func(addr string) error) {
	a.checkAddr = checkAddr
}

// updateAddress is a helper function to either update an address already known
// to the address manager, or to add the address if not already known.
func (a *AddrManager) updateAddress(netAddr, srcAddr *p2p.NetAddress) {
	// Filter out non-routable addresses. Note that non-routable
	// also includes invalid and local addresses.
	if !IsRoutable(netAddr) {
		log.Debugf("addr %v is non-routable", netAddr)
		return
	}

	addr := NetAddressKey(netAddr)
	ka := a.find(netAddr)
	if ka != nil {
		// TODO: only update addresses periodically.
		// Update the last seen time and services.
		// note that to prevent causing excess garbage on getaddr
		// messages the netaddresses in addrmaanger are *immutable*,
		// if we need to change them then we replace the pointer with a
		// new copy so that we don't have to copy every na for getaddr.
		if netAddr.Timestamp.After(ka.na.Timestamp) ||
			(ka.na.Services&netAddr.Services) !=
				netAddr.Services {

			naCopy := *ka.na
			naCopy.Timestamp = netAddr.Timestamp
			naCopy.AddService(netAddr.Services)
			ka.na = &naCopy
		}

		// If already in tried, we have nothing to do here.
		if ka.tried {
			return
		}

		// Already at our max?
		if ka.refs == newBucketsPerAddress {
			return
		}

		// The more entries we have, the less likely we are to add more.
		// likelihood is 2N.
		factor := int32(2 * ka.refs)
		if a.rand.Int31n(factor) != 0 {
			return
		}
	} else {
		// Make a copy of the net address to avoid races since it is
		// updated elsewhere in the addrmanager code and would otherwise
		// change the actual netaddress on the peer.
		netAddrCopy := *netAddr
		ka = &KnownAddress{na: &netAddrCopy, srcAddr: srcAddr}
		a.addrIndex[addr] = ka
		a.nNew++
		// XXX time penalty?
	}

	bucket := a.getNewBucket(netAddr, srcAddr)

	// Already exists?
	if _, ok := a.addrNew[bucket][addr]; ok {
		return
	}

	// Enforce max addresses.
	if len(a.addrNew[bucket]) > newBucketSize {
		a.expireNew(bucket)
	}

	// Add to new bucket.
	ka.refs++
	a.addrNew[bucket][addr] = ka
}

// expireNew makes space in the new buckets by expiring the really bad entries.
// If no bad entries are available we look at a few and remove the oldest.
func (a *AddrManager) expireNew(bucket int) {
	// First see if there are any entries that are so bad we can just throw
	// them away. otherwise we throw away the oldest entry in the cache.
	// Here chooses four random and just throws the oldest of
	// those away, but we keep track of oldest in the initial traversal and
	// use that information instead.
	var oldest *KnownAddress
	for k, v := range a.addrNew[bucket] {
		if v.isBad() {
			delete(a.addrNew[bucket], k)
			v.refs--
			if v.refs == 0 {
				a.nNew--
				delete(a.addrIndex, k)
			}
			continue
		}
		if oldest == nil {
			oldest = v
		} else if !v.na.Timestamp.After(oldest.na.Timestamp) {
			oldest = v
		}
	}

	if oldest != nil {
		key := NetAddressKey(oldest.na)

		delete(a.addrNew[bucket], key)
		oldest.refs--
		if oldest.refs == 0 {
			a.nNew--
			delete(a.addrIndex, key)
		}
	}
}

// pickTried selects an address from the tried bucket to be evicted.
// We just choose the eldest. Selects 4 random entries and throws away
// the older of them.
func (a *AddrManager) pickTried(bucket int) *list.Element {
	var oldest *KnownAddress
	var oldestElem *list.Element
	for e := a.addrTried[bucket].Front(); e != nil; e = e.Next() {
		ka := e.Value.(*KnownAddress)
		if oldest == nil || oldest.na.Timestamp.After(ka.na.Timestamp) {
			oldestElem = e
			oldest = ka
		}

	}
	return oldestElem
}

func (a *AddrManager) getNewBucket(netAddr, srcAddr *p2p.NetAddress) int {
	data1 := []byte{}
	data1 = append(data1, a.key[:]...)
	data1 = append(data1, []byte(GroupKey(netAddr))...)
	data1 = append(data1, []byte(GroupKey(srcAddr))...)
	hash1 := common.Sha256D(data1)
	hash64 := binary.LittleEndian.Uint64(hash1[:])
	hash64 %= newBucketsPerGroup
	var hashbuf [8]byte
	binary.LittleEndian.PutUint64(hashbuf[:], hash64)
	data2 := []byte{}
	data2 = append(data2, a.key[:]...)
	data2 = append(data2, GroupKey(srcAddr)...)
	data2 = append(data2, hashbuf[:]...)

	hash2 := common.Sha256D(data2)
	return int(binary.LittleEndian.Uint64(hash2[:]) % newBucketCount)
}

func (a *AddrManager) getTriedBucket(netAddr *p2p.NetAddress) int {
	data1 := []byte{}
	data1 = append(data1, a.key[:]...)
	data1 = append(data1, []byte(NetAddressKey(netAddr))...)
	hash1 := common.Sha256D(data1)
	hash64 := binary.LittleEndian.Uint64(hash1[:])
	hash64 %= triedBucketsPerGroup
	var hashbuf [8]byte
	binary.LittleEndian.PutUint64(hashbuf[:], hash64)
	data2 := []byte{}
	data2 = append(data2, a.key[:]...)
	data2 = append(data2, GroupKey(netAddr)...)
	data2 = append(data2, hashbuf[:]...)

	hash2 := common.Sha256D(data2)
	return int(binary.LittleEndian.Uint64(hash2[:]) % triedBucketCount)
}

// addressHandler is the main handler for the address manager.  It must be run
// as a goroutine.
func (a *AddrManager) addressHandler() {
	dumpAddressTicker := time.NewTicker(dumpAddressInterval)
	defer dumpAddressTicker.Stop()

	expireAddressTicker := time.NewTicker(expireAddressInterval)
	defer expireAddressTicker.Stop()
out:
	for {
		select {
		case <-dumpAddressTicker.C:
			a.savePeers()

		case <-expireAddressTicker.C:
			go a.expirePeers()

		case <-a.quit:
			break out
		}
	}
	a.savePeers()
	a.wg.Done()
}

// expirePeers remove all the expired addresses.
func (a *AddrManager) expirePeers() {
	a.mtx.Lock()
	addrList := a.copyAddrList()
	a.mtx.Unlock()

	log.Info("Start check addrList, count:", len(addrList))
	removeList := make(map[string]struct{})
	for k := range addrList {
		if err := a.checkAddr(k); err != nil {
			removeList[k] = struct{}{}
		}
	}
	log.Infof("Check addrList finished, need to remove %d addr, "+
		"%d addr is ok", len(removeList), len(addrList)-len(removeList))

	a.mtx.Lock()
	for k, _ := range removeList {
		var existInAddrNew bool
		for i := 0; i < newBucketCount; i++ {
			if _, ok := a.addrNew[i][k]; ok {
				a.addrNew[i][k].refs--
				delete(a.addrNew[i], k)
				existInAddrNew = true
			}
		}

		if existInAddrNew && a.addrIndex[k].refs == 0 {
			a.nNew--
			delete(a.addrIndex, k)
		}
	}
	a.mtx.Unlock()
}

func (a *AddrManager) copyAddrList() map[string]*KnownAddress {
	addrList := make(map[string]*KnownAddress)
	for k, v := range a.addrIndex {
		p := *v
		addrList[k] = &p
	}
	return addrList
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
	sam.Version = serialisationVersion
	copy(sam.Key[:], a.key[:])

	sam.Addresses = make([]*serializedKnownAddress, 0, len(a.addrIndex))
	for k, v := range a.addrIndex {
		// Filter network address here to update address saved in peers.json.
		if a.filter != nil && !a.filter.Filter(v.na) {
			continue
		}

		ska := new(serializedKnownAddress)
		ska.Addr = k
		ska.TimeStamp = v.na.Timestamp.Unix()
		ska.Src = NetAddressKey(v.srcAddr)
		ska.Attempts = v.attempts
		ska.LastAttempt = v.lastattempt.Unix()
		ska.LastSuccess = v.lastsuccess.Unix()
		// Tried and refs are implicit in the rest of the structure
		// and will be worked out from context on unserialisation.
		sam.Addresses = append(sam.Addresses, ska)
	}
	for i := range a.addrNew {
		sam.NewBuckets[i] = make([]string, 0, len(a.addrNew[i]))
		for k, v := range a.addrNew[i] {
			// Filter network address here to update address saved in peers.json.
			if a.filter != nil && !a.filter.Filter(v.na) {
				continue
			}
			sam.NewBuckets[i] = append(sam.NewBuckets[i], k)
		}
	}
	for i := range a.addrTried {
		sam.TriedBuckets[i] = make([]string, 0, a.addrTried[i].Len())
		for e := a.addrTried[i].Front(); e != nil; e = e.Next() {
			ka := e.Value.(*KnownAddress)
			// Filter network address here to update address saved in peers.json.
			if a.filter != nil && !a.filter.Filter(ka.na) {
				continue
			}
			sam.TriedBuckets[i] = append(sam.TriedBuckets[i], NetAddressKey(ka.na))
		}
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
		a.reset()
		return
	}
	log.Infof("Loaded %d addresses from file '%s'", a.numAddresses(), a.peersFile)
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

	if sam.Version != serialisationVersion {
		return fmt.Errorf("unknown version %v in serialized "+
			"addrmanager", sam.Version)
	}
	copy(a.key[:], sam.Key[:])

	for _, v := range sam.Addresses {
		ka := new(KnownAddress)
		ka.na, err = a.DeserializeNetAddress(v.Addr)
		if err != nil {
			return fmt.Errorf("failed to deserialize netaddress "+
				"%s: %v", v.Addr, err)
		}
		ka.srcAddr, err = a.DeserializeNetAddress(v.Src)
		if err != nil {
			return fmt.Errorf("failed to deserialize netaddress "+
				"%s: %v", v.Src, err)
		}
		ka.attempts = v.Attempts
		ka.lastattempt = time.Unix(v.LastAttempt, 0)
		ka.lastsuccess = time.Unix(v.LastSuccess, 0)
		a.addrIndex[NetAddressKey(ka.na)] = ka
	}

	for i := range sam.NewBuckets {
		for _, val := range sam.NewBuckets[i] {
			ka, ok := a.addrIndex[val]
			if !ok {
				return fmt.Errorf("newbucket contains %s but "+
					"none in address list", val)
			}

			if ka.refs == 0 {
				a.nNew++
			}
			ka.refs++
			a.addrNew[i][val] = ka
		}
	}
	for i := range sam.TriedBuckets {
		for _, val := range sam.TriedBuckets[i] {
			ka, ok := a.addrIndex[val]
			if !ok {
				return fmt.Errorf("Newbucket contains %s but "+
					"none in address list", val)
			}

			ka.tried = true
			a.nTried++
			a.addrTried[i].PushBack(ka)
		}
	}

	// Sanity checking.
	for k, v := range a.addrIndex {
		if v.refs == 0 && !v.tried {
			return fmt.Errorf("address %s after serialisation "+
				"with no references", k)
		}

		if v.refs > 0 && v.tried {
			return fmt.Errorf("address %s after serialisation "+
				"which is both new and tried!", k)
		}
	}

	return nil
}

// DeserializeNetAddress converts a given address string to a *p2p.NetAddress
func (a *AddrManager) DeserializeNetAddress(addr string) (*p2p.NetAddress, error) {
	host, portStr, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, err
	}
	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}

	// The default services were temporarily changed from 0 to 7 to support
	// address broadcasting after peers were read from peers.json file.
	return a.HostToNetAddress(host, uint16(port), 7)
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

// AddAddresses adds new addresses to the address manager.  It enforces a max
// number of addresses and silently ignores duplicate addresses.  It is
// safe for concurrent access.
func (a *AddrManager) AddAddresses(addrs []*p2p.NetAddress, srcAddr *p2p.NetAddress) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	for _, na := range addrs {
		a.updateAddress(na, srcAddr)
	}
}

// AddAddress adds a new address to the address manager.  It enforces a max
// number of addresses and silently ignores duplicate addresses.  It is
// safe for concurrent access.
func (a *AddrManager) AddAddress(addr, srcAddr *p2p.NetAddress) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	a.updateAddress(addr, srcAddr)
}

// AddAddressByIP adds an address where we are given an ip:port and not a
// p2p.NetAddress.
func (a *AddrManager) AddAddressByIP(addrIP string) error {
	// Split IP and port
	addr, portStr, err := net.SplitHostPort(addrIP)
	if err != nil {
		return err
	}
	// Put it in p2p.Netaddress
	ip := net.ParseIP(addr)
	if ip == nil {
		return fmt.Errorf("invalid ip address %s", addr)
	}
	port, err := strconv.ParseUint(portStr, 10, 0)
	if err != nil {
		return fmt.Errorf("invalid port %s: %v", portStr, err)
	}
	na := p2p.NewNetAddressIPPort(ip, uint16(port), 0)
	a.AddAddress(na, na) // XXX use correct src address
	return nil
}

// NumAddresses returns the number of addresses known to the address manager.
func (a *AddrManager) numAddresses() int {
	return a.nTried + a.nNew
}

// NumAddresses returns the number of addresses known to the address manager.
func (a *AddrManager) NumAddresses() int {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	return a.numAddresses()
}

// NeedMoreAddresses returns whether or not the address manager needs more
// addresses.
func (a *AddrManager) NeedMoreAddresses() bool {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	return a.numAddresses() < needAddressThreshold
}

// AddressCache returns the current address cache.  It must be treated as
// read-only (but since it is a copy now, this is not as dangerous).
func (a *AddrManager) AddressCache() []*p2p.NetAddress {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	addrIndexLen := len(a.addrIndex)
	if addrIndexLen == 0 {
		return nil
	}

	allAddr := make([]*p2p.NetAddress, 0, addrIndexLen)
	// Iteration order is undefined here, but we randomise it anyway.
	for _, v := range a.addrIndex {
		if a.filter == nil || a.filter.Filter(v.na) {
			allAddr = append(allAddr, v.na)
		}
	}
	addrIndexLen = len(allAddr)

	numAddresses := addrIndexLen * getAddrPercent / 100
	if numAddresses <= getAddrMin {
		numAddresses = min(getAddrMin, addrIndexLen)
	}
	if numAddresses > getAddrMax {
		numAddresses = getAddrMax
	}

	// Fisher-Yates shuffle the array. We only need to do the first
	// `numAddresses' since we are throwing the rest.
	for i := 0; i < numAddresses; i++ {
		// pick a number between current index and the end
		j := rand.Intn(addrIndexLen-i) + i
		allAddr[i], allAddr[j] = allAddr[j], allAddr[i]
	}

	// slice off the limit we are willing to share.
	return allAddr[0:numAddresses]
}

// reset resets the address manager by reinitialising the random source
// and allocating fresh empty bucket storage.
func (a *AddrManager) reset() {

	a.addrIndex = make(map[string]*KnownAddress)

	// fill key with bytes from a good random source.
	io.ReadFull(crand.Reader, a.key[:])
	for i := range a.addrNew {
		a.addrNew[i] = make(map[string]*KnownAddress)
	}
	a.nNew = 0

	for i := range a.addrTried {
		a.addrTried[i] = list.New()
	}
	a.nTried = 0
}

// HostToNetAddress returns a netaddress given a host address.  If the address
// is a Tor .onion address this will be taken care of.  Else if the host is
// not an IP address it will be resolved (via Tor if required).
func (a *AddrManager) HostToNetAddress(host string, port uint16, services uint64) (*p2p.NetAddress, error) {
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

	return p2p.NewNetAddressIPPort(ip, port, services), nil
}

// ipString returns a string for the ip from the provided NetAddress. If the
// ip is in the range used for Tor addresses then it will be transformed into
// the relevant .onion address.
func ipString(na *p2p.NetAddress) string {
	if IsOnionCatTor(na) {
		// We know now that na.IP is long enough.
		base32 := base32.StdEncoding.EncodeToString(na.IP[6:])
		return strings.ToLower(base32) + ".onion"
	}

	return na.IP.String()
}

// NetAddressKey returns a string key in the form of ip:port for IPv4 addresses
// or [ip]:port for IPv6 addresses.
func NetAddressKey(na *p2p.NetAddress) string {
	port := strconv.FormatUint(uint64(na.Port), 10)

	return net.JoinHostPort(ipString(na), port)
}

// GetAddress returns a single address that should be routable.  It picks a
// random one from the possible addresses with preference given to ones that
// have not been used recently and should not pick 'close' addresses
// consecutively.
func (a *AddrManager) GetAddress() *KnownAddress {
	// Protect concurrent access.
	a.mtx.Lock()
	defer a.mtx.Unlock()

	if a.numAddresses() == 0 {
		return nil
	}

	// Use a 50% chance for choosing between tried and new table entries.
	if a.nTried > 0 && (a.nNew == 0 || a.rand.Intn(2) == 0) {
		// Tried entry.
		large := 1 << 30
		factor := 1.0
		for {
			// pick a random bucket.
			bucket := a.rand.Intn(len(a.addrTried))
			if a.addrTried[bucket].Len() == 0 {
				continue
			}

			// Pick a random entry in the list
			e := a.addrTried[bucket].Front()
			for i :=
				a.rand.Int63n(int64(a.addrTried[bucket].Len())); i > 0; i-- {
				e = e.Next()
			}
			ka := e.Value.(*KnownAddress)
			randval := a.rand.Intn(large)
			if float64(randval) < (factor * ka.chance() * float64(large)) {
				return ka
			}
			factor *= 1.2
		}
	} else {
		// new node.
		// XXX use a closure/function to avoid repeating this.
		large := 1 << 30
		factor := 1.0
		for {
			// Pick a random bucket.
			bucket := a.rand.Intn(len(a.addrNew))
			if len(a.addrNew[bucket]) == 0 {
				continue
			}
			// Then, a random entry in it.
			var ka *KnownAddress
			nth := a.rand.Intn(len(a.addrNew[bucket]))
			for _, value := range a.addrNew[bucket] {
				if nth == 0 {
					ka = value
				}
				nth--
			}
			randval := a.rand.Intn(large)
			if float64(randval) < (factor * ka.chance() * float64(large)) {
				return ka
			}
			factor *= 1.2
		}
	}
}

func (a *AddrManager) find(addr *p2p.NetAddress) *KnownAddress {
	return a.addrIndex[NetAddressKey(addr)]
}

// Attempt increases the given address' attempt counter and updates
// the last attempt time.
func (a *AddrManager) Attempt(addr *p2p.NetAddress) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	// find address.
	// Surely address will be in tried by now?
	ka := a.find(addr)
	if ka == nil {
		return
	}
	// set last tried time to now
	ka.attempts++
	ka.lastattempt = time.Now()
}

// Connected Marks the given address as currently connected and working at the
// current time.  The address must already be known to AddrManager else it will
// be ignored.
func (a *AddrManager) Connected(addr *p2p.NetAddress) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	ka := a.find(addr)
	if ka == nil {
		return
	}

	// Update the time as long as it has been 20 minutes since last we did
	// so.
	now := time.Now()
	if now.After(ka.na.Timestamp.Add(time.Minute * 20)) {
		// ka.na is immutable, so replace it.
		naCopy := *ka.na
		naCopy.Timestamp = time.Now()
		ka.na = &naCopy
	}
}

// Good marks the given address as good.  To be called after a successful
// connection and version exchange.  If the address is unknown to the address
// manager it will be ignored.
func (a *AddrManager) Good(addr *p2p.NetAddress) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	ka := a.find(addr)
	if ka == nil {
		return
	}

	// ka.Timestamp is not updated here to avoid leaking information
	// about currently connected peers.
	now := time.Now()
	ka.lastsuccess = now
	ka.lastattempt = now
	ka.attempts = 0

	// move to tried set, optionally evicting other addresses if neeed.
	if ka.tried {
		return
	}

	// ok, need to move it to tried.

	// remove from all new buckets.
	// record one of the buckets in question and call it the `first'
	addrKey := NetAddressKey(addr)
	oldBucket := -1
	for i := range a.addrNew {
		// we check for existence so we can record the first one
		if _, ok := a.addrNew[i][addrKey]; ok {
			delete(a.addrNew[i], addrKey)
			ka.refs--
			if oldBucket == -1 {
				oldBucket = i
			}
		}
	}
	a.nNew--

	if oldBucket == -1 {
		// What? wasn't in a bucket after all.... Panic?
		return
	}

	bucket := a.getTriedBucket(ka.na)

	// Room in this tried bucket?
	if a.addrTried[bucket].Len() < triedBucketSize {
		ka.tried = true
		a.addrTried[bucket].PushBack(ka)
		a.nTried++
		return
	}

	// No room, we have to evict something else.
	entry := a.pickTried(bucket)
	rmka := entry.Value.(*KnownAddress)

	// First bucket it would have been put in.
	newBucket := a.getNewBucket(rmka.na, rmka.srcAddr)

	// If no room in the original bucket, we put it in a bucket we just
	// freed up a space in.
	if len(a.addrNew[newBucket]) >= newBucketSize {
		newBucket = oldBucket
	}

	// replace with ka in list.
	ka.tried = true
	entry.Value = ka

	rmka.tried = false
	rmka.refs++

	// We don't touch a.nTried here since the number of tried stays the same
	// but we decemented new above, raise it again since we're putting
	// something back.
	a.nNew++

	rmkey := NetAddressKey(rmka.na)

	// We made sure there is space here just above.
	a.addrNew[newBucket][rmkey] = rmka
}

// SetServices sets the services for the giiven address to the provided value.
func (a *AddrManager) SetServices(addr *p2p.NetAddress, services uint64) {
	a.mtx.Lock()
	defer a.mtx.Unlock()

	ka := a.find(addr)
	if ka == nil {
		return
	}

	// Update the services if needed.
	if ka.na.Services != services {
		// ka.na is immutable, so replace it.
		naCopy := *ka.na
		naCopy.Services = services
		ka.na = &naCopy
	}
}

// AddLocalAddress adds na to the list of known local addresses to advertise
// with the given priority.
func (a *AddrManager) AddLocalAddress(na *p2p.NetAddress, priority AddressPriority) error {
	if !IsRoutable(na) {
		return fmt.Errorf("address %s is not routable", na.IP)
	}

	a.lamtx.Lock()
	defer a.lamtx.Unlock()

	key := NetAddressKey(na)
	la, ok := a.localAddresses[key]
	if !ok || la.score < priority {
		if ok {
			la.score = priority + 1
		} else {
			a.localAddresses[key] = &localAddress{
				na:    na,
				score: priority,
			}
		}
	}
	return nil
}

// min returns the smaller of x or y.
func min(x, y int) int {
	if x < y {
		return x
	}
	return y
}

// getReachabilityFrom returns the relative reachability of the provided local
// address to the provided remote address.
func getReachabilityFrom(localAddr, remoteAddr *p2p.NetAddress) int {
	const (
		Unreachable = 0
		Default     = iota
		Teredo
		Ipv6Weak
		Ipv4
		Ipv6Strong
		Private
	)

	if !IsRoutable(remoteAddr) {
		return Unreachable
	}

	if IsOnionCatTor(remoteAddr) {
		if IsOnionCatTor(localAddr) {
			return Private
		}

		if IsRoutable(localAddr) && IsIPv4(localAddr) {
			return Ipv4
		}

		return Default
	}

	if IsRFC4380(remoteAddr) {
		if !IsRoutable(localAddr) {
			return Default
		}

		if IsRFC4380(localAddr) {
			return Teredo
		}

		if IsIPv4(localAddr) {
			return Ipv4
		}

		return Ipv6Weak
	}

	if IsIPv4(remoteAddr) {
		if IsRoutable(localAddr) && IsIPv4(localAddr) {
			return Ipv4
		}
		return Unreachable
	}

	/* ipv6 */
	var tunnelled bool
	// Is our v6 is tunnelled?
	if IsRFC3964(localAddr) || IsRFC6052(localAddr) || IsRFC6145(localAddr) {
		tunnelled = true
	}

	if !IsRoutable(localAddr) {
		return Default
	}

	if IsRFC4380(localAddr) {
		return Teredo
	}

	if IsIPv4(localAddr) {
		return Ipv4
	}

	if tunnelled {
		// only prioritise ipv6 if we aren't tunnelling it.
		return Ipv6Weak
	}

	return Ipv6Strong
}

// GetBestLocalAddress returns the most appropriate local address to use
// for the given remote address.
func (a *AddrManager) GetBestLocalAddress(remoteAddr *p2p.NetAddress) *p2p.NetAddress {
	a.lamtx.Lock()
	defer a.lamtx.Unlock()

	bestreach := 0
	var bestscore AddressPriority
	var bestAddress *p2p.NetAddress
	for _, la := range a.localAddresses {
		reach := getReachabilityFrom(la.na, remoteAddr)
		if reach > bestreach ||
			(reach == bestreach && la.score > bestscore) {
			bestreach = reach
			bestscore = la.score
			bestAddress = la.na
		}
	}
	if bestAddress != nil {
		log.Debugf("Suggesting address %s:%d for %s:%d", bestAddress.IP,
			bestAddress.Port, remoteAddr.IP, remoteAddr.Port)
	} else {
		log.Debugf("No worthy address for %s:%d", remoteAddr.IP,
			remoteAddr.Port)

		// Send something unroutable if nothing suitable.
		var ip net.IP
		if !IsIPv4(remoteAddr) && !IsOnionCatTor(remoteAddr) {
			ip = net.IPv6zero
		} else {
			ip = net.IPv4zero
		}
		bestAddress = p2p.NewNetAddressIPPort(ip, 0, 0)
	}

	return bestAddress
}

// New returns a new address manager.
// Use Start to begin processing asynchronous address updates.
func New(dataDir string, filter p2p.NAFilter) *AddrManager {
	am := AddrManager{
		filter:         filter,
		peersFile:      filepath.Join(dataDir, "peers.json"),
		rand:           rand.New(rand.NewSource(time.Now().UnixNano())),
		quit:           make(chan struct{}),
		localAddresses: make(map[string]*localAddress),
	}
	am.reset()
	return &am
}
