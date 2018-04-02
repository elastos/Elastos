package sdk

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

// This is a utils class to help filter interest addresses
type AddrFilter struct {
	sync.Mutex
	addrs map[Uint168]*Uint168
}

// Create a AddrFilter instance, you can pass all the addresses this method
// or pass nil and user AddAddr() method to add interested addresses later.
func NewAddrFilter(addrs []*Uint168) *AddrFilter {
	filter := new(AddrFilter)
	filter.LoadAddrs(addrs)
	return filter
}

// Load or reload all the interested addresses into the AddrFilter
func (filter *AddrFilter) LoadAddrs(addrs []*Uint168) {
	filter.Lock()
	defer filter.Unlock()

	filter.addrs = make(map[Uint168]*Uint168)
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
func (filter *AddrFilter) AddAddr(addr *Uint168) {
	filter.Lock()
	defer filter.Unlock()

	filter.addrs[*addr] = addr
}

// Remove an address from this Filter
func (filter *AddrFilter) DeleteAddr(hash Uint168) {
	filter.Lock()
	defer filter.Unlock()

	delete(filter.addrs, hash)
}

// Get addresses that were added into this Filter
func (filter *AddrFilter) GetAddrs() []*Uint168 {
	var scriptHashes = make([]*Uint168, 0, len(filter.addrs))
	for scriptHash := range filter.addrs {
		scriptHashes = append(scriptHashes, &scriptHash)
	}

	return scriptHashes
}

// Check if an address was added into this filter as a interested address
func (filter *AddrFilter) ContainAddr(hash Uint168) bool {
	filter.Lock()
	defer filter.Unlock()

	_, ok := filter.addrs[hash]
	return ok
}
