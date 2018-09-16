package node

import (
	"fmt"
	"sync"

	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// The neigbor Node list
type nbrNodes struct {
	sync.RWMutex
	// Todo using the Pool structure
	List map[uint64]*Node
}

func (nm *nbrNodes) NodeExisted(uid uint64) bool {
	_, ok := nm.List[uid]
	return ok
}

func (nm *nbrNodes) AddNbrNode(n Noder) {
	nm.Lock()
	defer nm.Unlock()

	if nm.NodeExisted(n.ID()) {
		fmt.Printf("Insert a existed Node\n")
	} else {
		node, err := n.(*Node)
		if err == false {
			fmt.Println("Convert the noder error when add Node")
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
		if node.State() == p2p.ESTABLISH {
			cnt++
		}
	}
	return cnt
}

func (nm *nbrNodes) init() {
	nm.List = make(map[uint64]*Node)
}

func (nm *nbrNodes) NodeEstablished(id uint64) bool {
	nm.RLock()
	defer nm.RUnlock()

	n, ok := nm.List[id]
	if ok == false {
		return false
	}

	if n.State() != p2p.ESTABLISH {
		return false
	}

	return true
}

func (node *Node) GetNeighborAddrs() []p2p.NetAddress {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	var addrs []p2p.NetAddress
	for _, n := range node.nbrNodes.List {
		if n.State() != p2p.ESTABLISH {
			continue
		}
		var addr p2p.NetAddress
		addr.IP, _ = n.Addr16()
		addr.Time = n.GetTime()
		addr.Services = n.Services()
		addr.Port = n.Port()
		addr.ID = n.ID()
		addrs = append(addrs, addr)
	}

	return addrs
}

func (node *Node) GetNeighborHeights() []uint64 {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	var heights []uint64
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.ESTABLISH {
			heights = append(heights, n.Height())
		}
	}
	return heights
}

func (node *Node) GetNeighborNoder() []Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	var nodes []Noder
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.ESTABLISH {
			node := n
			nodes = append(nodes, node)
		}
	}
	return nodes
}

func (node *Node) GetNbrNodeCnt() uint32 {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	var count uint32
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.ESTABLISH {
			count++
		}
	}
	return count
}

func (node *Node) RandGetANbr() Noder {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()
	for _, n := range node.nbrNodes.List {
		if n.State() == p2p.ESTABLISH {
			return n
		}
	}
	return nil
}

func (node *Node) IsNeighborNoder(n Noder) bool {
	node.nbrNodes.RLock()
	defer node.nbrNodes.RUnlock()

	for _, noder := range node.nbrNodes.List {
		if n.ID() == noder.ID() {
			return true
		}
	}
	return false
}
