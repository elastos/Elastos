package node

import (
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// The neighbor node list
type neighbours struct {
	sync.Mutex
	List map[uint64]protocol.Noder
}

func (ns *neighbours) init() {
	ns.List = make(map[uint64]protocol.Noder)
}

func (ns *neighbours) AddNeighborNode(node protocol.Noder) {
	ns.Lock()
	defer ns.Unlock()
	ns.List[node.ID()] = node
}

func (ns *neighbours) DelNeighborNode(id uint64) (protocol.Noder, bool) {
	ns.Lock()
	defer ns.Unlock()
	node, ok := ns.List[id]
	if ok == false {
		return nil, false
	}
	delete(ns.List, id)
	return node, true
}

func (ns *neighbours) IsNeighborAddr(addr string) bool {
	ns.Lock()
	defer ns.Unlock()
	for _, n := range ns.List {
		if n.State() == protocol.ESTABLISHED {
			if n.NetAddress().String() == addr {
				return true
			}
		}
	}
	return false
}

func (ns *neighbours) GetConnectionCount() (internal uint, total uint) {
	ns.Lock()
	defer ns.Unlock()
	for _, node := range ns.List {
		// Skip unestablished nodes
		if node.State() != protocol.ESTABLISHED {
			continue
		}

		// Count internal nodes
		if !node.IsExternal() {
			internal++
		}

		// Count total nodes
		total++
	}

	return internal, total
}

func (ns *neighbours) NodeEstablished(id uint64) bool {
	ns.Lock()
	defer ns.Unlock()

	node, ok := ns.List[id]
	if ok == false {
		return false
	}

	if node.State() != protocol.ESTABLISHED {
		return false
	}

	return true
}

func (ns *neighbours) GetNeighbourAddresses() []*p2p.NetAddress {
	ns.Lock()
	defer ns.Unlock()

	var addrs []*p2p.NetAddress
	for _, n := range ns.List {
		if n.State() != protocol.ESTABLISHED {
			continue
		}
		addrs = append(addrs, n.NetAddress())
	}

	return addrs
}

func (ns *neighbours) GetInternalNeighborHeights() []uint64 {
	neighbors := ns.GetNeighborNodes()

	heights := make([]uint64, 0, len(neighbors))
	for _, n := range neighbors {
		if n.State() == protocol.ESTABLISHED && !n.IsExternal(){
			height := n.Height()
			heights = append(heights, height)
		}
	}

	return heights
}

func (ns *neighbours) GetNeighborNodes() []protocol.Noder {
	ns.Lock()
	defer ns.Unlock()

	nodes := make([]protocol.Noder, 0, len(ns.List))
	for _, n := range ns.List {
		if n.State() == protocol.ESTABLISHED {
			node := n
			nodes = append(nodes, node)
		}
	}

	// Sort by node id before return
	sort.Slice(nodes, func(i, j int) bool {
		return nodes[i].ID() < nodes[j].ID()
	})

	return nodes
}

func (ns *neighbours) GetNeighbourCount() uint {
	_, count := ns.GetConnectionCount()
	return count
}

func (ns *neighbours) GetExternalNeighbourRandomly() protocol.Noder {
	ns.Lock()
	defer ns.Unlock()
	for _, n := range ns.List {
		if n.State() == protocol.ESTABLISHED && n.IsExternal(){
			return n
		}
	}
	return nil
}

func (ns *neighbours) IsNeighborNode(id uint64) bool {
	ns.Lock()
	defer ns.Unlock()
	_, ok := ns.List[id]
	return ok
}

func (ns *neighbours) GetSyncNode() protocol.Noder {
	ns.Lock()
	defer ns.Unlock()
	for _, n := range ns.List {
		if n.IsSyncHeaders() {
			return n
		}
	}
	return nil
}

func (ns *neighbours) GetBestNode() protocol.Noder {
	ns.Lock()
	defer ns.Unlock()

	var best protocol.Noder
	for _, nbr := range ns.List {
		// Do not let external node become sync node
		if nbr.State() != protocol.ESTABLISHED || nbr.IsExternal() {
			continue
		}

		if best == nil {
			best = nbr
			continue
		}

		if nbr.Height() > best.Height() {
			best = nbr
		}
	}

	return best
}
