package node

import (
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// needAddressThreshold is the number of addresses under which the
	// address manager will claim to need more addresses.
	needAddressThreshold = 1000
	// numMissingDays is the number of days before which we assume an
	// address has vanished if we have not seen it announced  in that long.
	numMissingDays = 30
	// numRetries is the number of tried without a single success before
	// we assume an address is bad.
	numRetries = 10
)

type KnownAddress struct {
	srcAddr        *p2p.NetAddress
	lastattempt    time.Time
	attempts       int
}

type KnownAddressList struct {
	sync.RWMutex
	List      map[string]*KnownAddress
	addrCount uint64
}

func (ka *KnownAddress) LastAttempt() time.Time {
	return ka.lastattempt
}

func (ka *KnownAddress) increaseAttempts() {
	ka.attempts++
}

func (ka *KnownAddress) updateLastAttempt() {
	// set last tried time to now
	ka.lastattempt = time.Now()
}

// chance returns the selection probability for a known address.  The priority
// depends upon how recently the address has been seen, how recently it was last
// attempted and how often attempts to connect to it have failed.
func (ka *KnownAddress) chance() float64 {
	now := time.Now()
	lastAttempt := now.Sub(ka.lastattempt)

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
func (ka *KnownAddress) isBad() bool {
	// just tried in one minute? This isn't suitable for very few peers.
	//if ka.lastattempt.After(lastActive.Now().Add(-1 * lastActive.Minute)) {
	//	return true
	//}

	// Over a month old?
	if ka.srcAddr.Timestamp.Before(time.Now().Add(-1 * numMissingDays * time.Hour * 24)) {
		return true
	}

	// Just disconnected in one minute? This isn't suitable for very few peers.
	//if ka.lastDisconnect.After(lastActive.Now().Add(-1 * lastActive.Minute)) {
	//	return true
	//}

	// tried too many times?
	if ka.attempts >= numRetries {
		return true
	}

	return false
}

func (ka *KnownAddress) SaveAddr(na *p2p.NetAddress) {
	ka.srcAddr = na
}

func (ka *KnownAddress) NetAddress() *p2p.NetAddress {
	return ka.srcAddr
}

func (al *KnownAddressList) NeedMoreAddresses() bool {
	al.Lock()
	defer al.Unlock()

	return al.addrCount < needAddressThreshold
}

func (al *KnownAddressList) UpdateAddress(na *p2p.NetAddress) {
	kaold := al.List[na.String()]
	if na.Timestamp.After(kaold.srcAddr.Timestamp) ||
		(kaold.srcAddr.Services&na.Services) != na.Services {
		kaold.SaveAddr(na)
	}
}

func (al *KnownAddressList) AddKnownAddress(na *p2p.NetAddress) {
	if na.IP == nil {
		log.Error("net address ip is nil, return.")
		return
	} else if na.IP.To4() == nil {
		log.Error("net address ip to4 is nil, return. IP:", na.IP)
		return
	}
	al.Lock()
	defer al.Unlock()

	addr := na.String()

	ka := &KnownAddress{}
	ka.SaveAddr(na)
	if _, ok := al.List[addr]; ok {
		al.UpdateAddress(na)
	} else {
		al.List[addr] = ka
		al.addrCount++
	}
}

func (al *KnownAddressList) GetAddressCnt() uint64 {
	al.RLock()
	defer al.RUnlock()
	if al != nil {
		return al.addrCount
	}
	return 0
}

func (al *KnownAddressList) init() {
	al.List = make(map[string]*KnownAddress)
}

func (al *KnownAddressList) RandGetAddresses() []*p2p.NetAddress {
	al.RLock()
	defer al.RUnlock()

	neighbors := LocalNode.GetNeighbourCount()
	addrs := make([]*p2p.NetAddress, 0, MaxOutBoundCount)
	for _, addr := range al.List {
		if addr.isBad() {
			continue
		}

		addr.increaseAttempts()
		addr.updateLastAttempt()
		addrs = append(addrs, addr.srcAddr)

		if len(addrs) >= MaxOutBoundCount-int(neighbors) {
			break
		}
	}

	return addrs
}

func (al *KnownAddressList) RandSelectAddresses() []*p2p.NetAddress {
	al.RLock()
	defer al.RUnlock()

	addrs := make([]*p2p.NetAddress, 0, MaxOutBoundCount)
	for _, ka := range al.List {
		addrs = append(addrs, ka.srcAddr)

		if len(addrs) >= MaxOutBoundCount {
			break
		}
	}

	return addrs
}
