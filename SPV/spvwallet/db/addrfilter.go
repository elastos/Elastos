package db

import (
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

type AddrFilter struct {
	sync.Mutex
	addrs map[Uint168]*Addr
}

func NewAddrFilter(addrs []*Addr) *AddrFilter {
	filter := new(AddrFilter)
	filter.LoadAddrs(addrs)
	return filter
}

func (filter *AddrFilter) LoadAddrs(addrs []*Addr) {
	filter.Lock()
	defer filter.Unlock()

	filter.addrs = make(map[Uint168]*Addr)
	for _, addr := range addrs {
		filter.addrs[*addr.Hash()] = addr
	}
}

func (filter *AddrFilter) IsLoaded() bool {
	return len(filter.addrs) > 0
}

func (filter *AddrFilter) AddAddr(addr *Addr) {
	filter.Lock()
	defer filter.Unlock()

	filter.addrs[*addr.Hash()] = addr
}

func (filter *AddrFilter) DeleteAddr(hash Uint168) {
	filter.Lock()
	defer filter.Unlock()

	delete(filter.addrs, hash)
}

func (filter *AddrFilter) GetScriptHashes() []Uint168 {
	var scriptHashes = make([]Uint168, 0)
	for scriptHash := range filter.addrs {
		scriptHashes = append(scriptHashes, scriptHash)
	}

	return scriptHashes
}

func (filter *AddrFilter) ContainAddress(hash Uint168) bool {
	filter.Lock()
	defer filter.Unlock()

	_, ok := filter.addrs[hash]
	return ok
}
