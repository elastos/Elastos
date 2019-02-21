package node

import (
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/protocol"
)

var _ p2p.NAFilter = (*nodeNAFilter)(nil)

// nodeNAFilter defines a filter to filter full node addresses.
type nodeNAFilter struct {}

// Returns true if network address is a full node.
func (f *nodeNAFilter) Filter(na *p2p.NetAddress) bool {
	return na.Services&protocol.FlagNode == protocol.FlagNode
}
