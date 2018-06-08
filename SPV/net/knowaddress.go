package net

import (
	"time"
	"sort"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// numMissingDays is the number of days before which we assume an
	// address has vanished if we have not seen it announced  in that long.
	numMissingDays = 30
)

type knownAddress struct {
	p2p.NetAddress
	lastAttempt    time.Time
	lastDisconnect time.Time
	attempts       uint32
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

// isBad returns true if the address in question has not been seen in over a month
func (ka *knownAddress) isBad() bool {
	// Over a month old?
	if ka.Time < time.Now().Add(-1 * numMissingDays * time.Hour * 24).UnixNano() {
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
func (c OrderByChance) Len() int { return len(c) }

// Less reports whether the element with
// index i should sort before the element with index j.
func (c OrderByChance) Less(i, j int) bool { return c[i].chance() > c[j].chance() }

// Swap swaps the elements with indexes i and j.
func (c OrderByChance) Swap(i, j int) { c[i], c[j] = c[j], c[i] }

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
