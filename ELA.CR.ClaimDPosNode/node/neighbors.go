package node

import (
	"sort"
	"sync"

	"github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA/p2p"
)

// The neighbor node list
type neighbours struct {
	sync.Mutex
	List map[protocol.Noder]struct{}
}

func (ns *neighbours) init() {
	ns.List = make(map[protocol.Noder]struct{})
}

func (ns *neighbours) AddNeighborNode(node protocol.Noder) {
	ns.Lock()
	defer ns.Unlock()
	ns.List[node] = struct{}{}
}

func (ns *neighbours) DelNeighborNode(node protocol.Noder) (protocol.Noder, bool) {
	ns.Lock()
	defer ns.Unlock()
	_, ok := ns.List[node]
	if ok == false {
		return nil, false
	}
	delete(ns.List, node)
	return node, true
}

func (ns *neighbours) IsNeighborAddr(addr string) bool {
	ns.Lock()
	defer ns.Unlock()
	for n := range ns.List {
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
	for node := range ns.List {
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

func (ns *neighbours) GetNeighbourAddresses() []*p2p.NetAddress {
	ns.Lock()
	defer ns.Unlock()

	var addrs []*p2p.NetAddress
	for n := range ns.List {
		if n.State() != protocol.ESTABLISHED {
			continue
		}
		addrs = append(addrs, n.NetAddress())
	}

	return addrs
}

func (ns *neighbours) GetInternalNeighborAddressAndHeights() ([]string, []uint64) {
	neighbors := ns.GetNeighborNodes()

	heights := make([]uint64, 0, len(neighbors))
	addresses := make([]string, 0, len(neighbors))
	for _, n := range neighbors {
		if n.State() == protocol.ESTABLISHED && !n.IsExternal() {
			height := n.Height()
			address := n.Addr()
			heights = append(heights, height)
			addresses = append(addresses, address)
		}
	}

	return addresses, heights
}

func (ns *neighbours) GetNeighborNodes() []protocol.Noder {
	ns.Lock()
	defer ns.Unlock()

	nodes := make([]protocol.Noder, 0, len(ns.List))
	for n := range ns.List {
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
	for n := range ns.List {
		if n.State() == protocol.ESTABLISHED && n.IsExternal() {
			return n
		}
	}
	return nil
}

func (ns *neighbours) IsNeighborNode(node protocol.Noder) bool {
	ns.Lock()
	defer ns.Unlock()
	_, ok := ns.List[node]
	return ok
}

func (ns *neighbours) GetSyncNode() protocol.Noder {
	ns.Lock()
	defer ns.Unlock()
	for n := range ns.List {
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
	for nbr := range ns.List {
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
