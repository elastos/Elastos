package node

import (
	"sync"

	"github.com/elastos/Elastos.ELA/net/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type idCache struct {
	sync.RWMutex
	lastid    Uint256
	index     int
	idarray   []Uint256
	idmaplsit map[Uint256]int
}

func (c *idCache) init() {
	c.index = 0
	c.idmaplsit = make(map[Uint256]int, protocol.MAXIDCACHED)
	c.idarray = make([]Uint256, protocol.MAXIDCACHED)
}

func (c *idCache) add(id Uint256) {
	oldid := c.idarray[c.index]
	delete(c.idmaplsit, oldid)
	c.idarray[c.index] = id
	c.idmaplsit[id] = c.index
	c.index++
	c.lastid = id
	c.index = c.index % protocol.MAXIDCACHED
}

func (c *idCache) ExistedID(id Uint256) bool {
	// TODO
	c.Lock()
	defer c.Unlock()
	if id == c.lastid {
		return true
	}
	if _, ok := c.idmaplsit[id]; ok {
		return true
	} else {
		c.add(id)
	}
	return false
}
