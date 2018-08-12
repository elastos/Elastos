package node

import (
	"sync"
	"sort"

	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// The neighbor node list
type neighbourNodes struct {
	sync.RWMutex
	List map[uint64]protocol.Noder
}

func (nm *neighbourNodes) init() {
	nm.List = make(map[uint64]protocol.Noder)
}

func (nm *neighbourNodes) AddNeighborNode(n protocol.Noder) {
	nm.Lock()
	defer nm.Unlock()

	nm.List[n.ID()] = n
}

func (nm *neighbourNodes) DelNeighborNode(id uint64) (protocol.Noder, bool) {
	nm.Lock()
	defer nm.Unlock()

	n, ok := nm.List[id]
	if ok == false {
		return nil, false
	}
	delete(nm.List, id)
	return n, true
}

func (nm *neighbourNodes) GetConnectionCount() (internal uint, total uint) {
	nm.RLock()
	defer nm.RUnlock()

	for _, node := range nm.List {
		// Skip unestablished nodes
		if node.State() != p2p.ESTABLISH {
			continue
		}

		// Count internal nodes
		if !node.IsFromExtraNet() {
			internal++
		}

		// Count total nodes
		total++
	}

	return internal, total
}

func (nm *neighbourNodes) NodeEstablished(id uint64) bool {
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

func (node *node) GetNeighbourAddresses() []p2p.NetAddress {
	node.neighbourNodes.RLock()
	defer node.neighbourNodes.RUnlock()

	var addrs []p2p.NetAddress
	for _, n := range node.neighbourNodes.List {
		if n.State() != p2p.ESTABLISH {
			continue
		}
		addrs = append(addrs, n.NetAddress())
	}

	return addrs
}

func (node *node) GetNeighborHeights() []uint64 {
	neighbors := node.GetNeighborNoder()
	heights := make([]uint64, 0, len(neighbors))
	for _, n := range neighbors {
		if n.State() == p2p.ESTABLISH {
			height := n.Height()
			heights = append(heights, height)
		}
	}

	return heights
}

func (node *node) GetNeighborNoder() []protocol.Noder {
	node.neighbourNodes.RLock()
	defer node.neighbourNodes.RUnlock()

	nodes := make([]protocol.Noder, 0, len(node.neighbourNodes.List))
	for _, n := range node.neighbourNodes.List {
		if n.State() == p2p.ESTABLISH {
			node := n
			nodes = append(nodes, node)
		}
	}

	// Sort by node id before return
	sort.Sort(nodeById(nodes))

	return nodes
}

func (node *node) GetNeighbourCount() uint32 {
	node.neighbourNodes.RLock()
	defer node.neighbourNodes.RUnlock()
	var count uint32
	for _, n := range node.neighbourNodes.List {
		if n.State() == p2p.ESTABLISH {
			count++
		}
	}
	return count
}

func (node *node) GetANeighbourRandomly() protocol.Noder {
	node.neighbourNodes.RLock()
	defer node.neighbourNodes.RUnlock()
	for _, n := range node.neighbourNodes.List {
		if n.State() == p2p.ESTABLISH {
			return n
		}
	}
	return nil
}

func (node *node) IsNeighborNoder(id uint64) bool {
	node.neighbourNodes.RLock()
	defer node.neighbourNodes.RUnlock()

	_, ok := node.neighbourNodes.List[id]
	return ok
}

type nodeById []protocol.Noder

func (ns nodeById) Len() int           { return len(ns) }
func (ns nodeById) Less(i, j int) bool { return ns[i].ID() < ns[j].ID() }
func (ns nodeById) Swap(i, j int)      { ns[i], ns[j] = ns[j], ns[i] }
