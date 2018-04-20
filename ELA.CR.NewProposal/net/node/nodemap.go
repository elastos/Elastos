package node

import (
	"fmt"
	"sync"

	. "github.com/elastos/Elastos.ELA/net/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

// The neigbor node list
type nbrNodes struct {
	sync.RWMutex
	// Todo using the Pool structure
	List map[uint64]*node
}

func (nm *nbrNodes) NodeExisted(uid uint64) bool {
	_, ok := nm.List[uid]
	return ok
}

func (nm *nbrNodes) AddNbrNode(n Noder) {
	nm.Lock()
	defer nm.Unlock()

	if nm.NodeExisted(n.ID()) {
		fmt.Printf("Insert a existed node\n")
	} else {
		node, err := n.(*node)
		if err == false {
			fmt.Println("Convert the noder error when add node")
			return
		}
		nm.List[n.ID()] = node
	}
}

func (nm *nbrNodes) DelNbrNode(id uint64) (Noder, bool) {
	nm.Lock()
	defer nm.Unlock()

	n, ok := nm.List[id]
	if ok == false {
		return nil, false
	}
	delete(nm.List, id)
	return n, true
}

func (nm *nbrNodes) GetConnectionCnt() uint {
	nm.RLock()
	defer nm.RUnlock()

	var cnt uint
	for _, node := range nm.List {
		if node.State() == ESTABLISH {
			cnt++
		}
	}
	return cnt
}

func (nm *nbrNodes) init() {
	nm.List = make(map[uint64]*node)
}

func (nm *nbrNodes) NodeEstablished(id uint64) bool {
	nm.RLock()
	defer nm.RUnlock()

	n, ok := nm.List[id]
	if ok == false {
		return false
	}

	if n.State() != ESTABLISH {
		return false
	}

	return true
}

func (node *node) GetNeighborAddrs() ([]msg.Addr, uint64) {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	var i uint64
	var addrs []msg.Addr
	for _, n := range node.nbrNodes.List {
		if n.State() != ESTABLISH {
			continue
		}
		var addr msg.Addr
		addr.IP, _ = n.Addr16()
		addr.Time = n.GetTime()
		addr.Services = n.Services()
		addr.Port = n.Port()
		addr.ID = n.ID()
		addrs = append(addrs, addr)

		i++
	}

	return addrs, i
}

func (node *node) GetNeighborHeights() ([]uint64, uint64) {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	var i uint64
	heights := []uint64{}
	for _, n := range node.nbrNodes.List {
		if n.State() == ESTABLISH {
			height := n.Height()
			heights = append(heights, height)
			i++
		}
	}
	return heights, i
}

func (node *node) GetNeighborNoder() []Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	nodes := []Noder{}
	for _, n := range node.nbrNodes.List {
		if n.State() == ESTABLISH {
			node := n
			nodes = append(nodes, node)
		}
	}
	return nodes
}

func (node *node) GetNbrNodeCnt() uint32 {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	var count uint32
	for _, n := range node.nbrNodes.List {
		if n.State() == ESTABLISH {
			count++
		}
	}
	return count
}

func (node *node) RandGetANbr() Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	for _, n := range node.nbrNodes.List {
		if n.State() == ESTABLISH {
			return n
		}
	}
	return nil
}

func (node *node) IsNeighborNoder(n Noder) bool {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	for _, noder := range node.nbrNodes.List {
		if n.ID() == noder.ID() {
			return true
		}
	}
	return false
}
