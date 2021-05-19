package sdk

import (
	"sync"

	"github.com/elastos/Elastos.ELA/common"
)

/*
This is a helper class to filter interested addresses when synchronize transactions
or get cached addresses list to build a bloom filter instead of load addresses from database every time.
*/
type AddrFilter struct {
	sync.Mutex
	addrs map[common.Uint168]*common.Uint168
}

// Create a AddrFilter instance, you can pass all the addresses through this method
// or pass nil and use AddAddr() method to add interested addresses later.
func NewAddrFilter(addrs []*common.Uint168) *AddrFilter {
	filter := new(AddrFilter)
	filter.LoadAddrs(addrs)
	return filter
}

// Load or reload all the interested addresses into the AddrFilter
func (filter *AddrFilter) LoadAddrs(addrs []*common.Uint168) {
	filter.Lock()
	defer filter.Unlock()

	filter.clear()
	for _, addr := range addrs {
		filter.addrs[*addr] = addr
	}
}

// Check if addresses are loaded into this Filter
func (filter *AddrFilter) IsLoaded() bool {
	filter.Lock()
	defer filter.Unlock()

	return len(filter.addrs) > 0
}

// Add a interested address into this Filter
func (filter *AddrFilter) AddAddr(addr *common.Uint168) {
	filter.Lock()
	defer filter.Unlock()

	filter.addrs[*addr] = addr
}

// Remove an address from this Filter
func (filter *AddrFilter) DeleteAddr(hash common.Uint168) {
	filter.Lock()
	defer filter.Unlock()

	delete(filter.addrs, hash)
}

// Get addresses that were added into this Filter
func (filter *AddrFilter) GetAddrs() []*common.Uint168 {
	filter.Lock()
	defer filter.Unlock()

	var addrs = make([]*common.Uint168, 0, len(filter.addrs))
	for _, addr := range filter.addrs {
		addrs = append(addrs, addr)
	}

	return addrs
}

// Check if an address was added into this filter as a interested address
func (filter *AddrFilter) ContainAddr(hash common.Uint168) bool {
	filter.Lock()
	defer filter.Unlock()

	_, ok := filter.addrs[hash]
	return ok
}

func (filter *AddrFilter) Clear() {
	filter.Lock()
	defer filter.Unlock()

	filter.clear()
}

func (filter *AddrFilter) clear() {
	filter.addrs = make(map[common.Uint168]*common.Uint168)
}
