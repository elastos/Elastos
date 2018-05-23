package net

import (
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"net"
	"strings"
	"strconv"
	"sort"
)

const (
	// numMissingDays is the number of days before which we assume an
	// address has vanished if we have not seen it announced  in that long.
	numMissingDays = 30
	// numRetries is the number of tried without a single success before
	// we assume an address is bad.
	numRetries = 10
)

type knownAddress struct {
	p2p.NetAddress
	lastAttempt    time.Time
	lastDisconnect time.Time
	attempts       uint32
}

func NewKnownAddress(addr string) *knownAddress {
	ka := new(knownAddress)
	portIndex := strings.LastIndex(addr, ":")
	if portIndex < 0 {
		return nil
	}
	copy(ka.IP[:], net.ParseIP(string([]byte(addr)[:portIndex])).To16())
	port, err := strconv.ParseUint(string([]byte(addr)[portIndex+1:]), 10, 16)
	if err != nil {
		return nil
	}
	ka.Port = uint16(port)
	ka.Time = time.Now().UnixNano()
	return ka
}

func (ka *knownAddress) LastAttempt() time.Time {
	return ka.lastAttempt
}

func (ka *knownAddress) increaseAttempts() {
	ka.attempts++
}

func (ka *knownAddress) updateLastAttempt() {
	// set last tried time to now
	ka.lastAttempt = time.Now()
}

func (ka *knownAddress) updateLastDisconnect() {
	// set last disconnect time to now
	ka.lastDisconnect = time.Now()
}

// chance returns the selection probability for a known address.  The priority
// depends upon how recently the address has been seen, how recently it was last
// attempted and how often attempts to connect to it have failed.
func (ka *knownAddress) chance() float64 {
	lastAttempt := time.Now().Sub(ka.lastAttempt)

	if lastAttempt < 0 {
		lastAttempt = 0
	}

	c := 1.0

	// Very recent attempts are less likely to be retried.
	if lastAttempt < 10*time.Minute {
		c *= 0.01
	}

	// Failed attempts deprioritise.
	for i := ka.attempts; i > 0; i-- {
		c /= 1.5
	}

	return c
}

// isBad returns true if the address in question has not been tried in the last
// minute and meets one of the following criteria:
// 1) Just tried in one minute
// 2) It hasn't been seen in over a month
// 3) It has failed at least ten times
// 4) It has failed ten times in the last week
// All addresses that meet these criteria are assumed to be worthless and not
// worth keeping hold of.
func (ka *knownAddress) isBad() bool {
	// Over a month old?
	if ka.Time < (time.Now().Add(-1 * numMissingDays * time.Hour * 24)).UnixNano() {
		return true
	}

	// tried too many times?
	if ka.attempts > numRetries {
		return true
	}

	return false
}

func (ka *knownAddress) SaveAddr(na *p2p.NetAddress) {
	ka.ID = na.ID
	ka.IP = na.IP
	ka.Port = na.Port
	ka.Services = na.Services
	ka.Time = na.Time
}

type OrderByChance []*knownAddress

// Len is the number of elements in the collection.
func (c OrderByChance) Len() int {
	return len(c)
}

// Less reports whether the element with
// index i should sort before the element with index j.
func (c OrderByChance) Less(i, j int) bool {
	return c[i].chance() > c[j].chance()
}

// Swap swaps the elements with indexes i and j.
func (c OrderByChance) Swap(i, j int) {
	chance := c[i]
	c[i] = c[j]
	c[j] = chance
}

func SortAddressMap(addrMap map[string]*knownAddress) []*knownAddress {
	var addrList []*knownAddress
	for _, addr := range addrMap {
		addrList = append(addrList, addr)
	}
	return SortAddressList(addrList)
}

func SortAddressList(addrList []*knownAddress) []*knownAddress {
	sort.Sort(OrderByChance(addrList))
	return addrList
}
