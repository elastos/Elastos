package pact

import (
	"strconv"
	"strings"
)

// Release version numbers
const (
	// The Release_0.0.1 version number of side chain
	Release001Version = 1

	// EBIP002Version is the protocol version which start support
	// transaction filtering through txfilter message.
	EBIP002Version uint32 = 10002
)

// ServiceFlag identifies services supported by a peer.
type ServiceFlag uint64

const (
	// SFNodeNetwork is a flag used to indicate a peer is a full node.
	SFNodeNetwork ServiceFlag = 1 << iota

	// SFNodeBloom is a flag used to indicate a peer supports bloom
	// filtering.
	SFNodeBloom

	// SFOpenService is a flag used to indicate a peer provide open service.
	SFOpenService

	// SFTxFiltering is a flag used to indicate a peer supports transaction
	// filtering.
	SFTxFiltering
)

// Map of service flags back to their constant names for pretty printing.
var sfStrings = map[ServiceFlag]string{
	SFNodeNetwork: "SFNodeNetwork",
	SFNodeBloom:   "SFNodeBloom",
	SFOpenService: "SFOpenService",
	SFTxFiltering: "SFTxFiltering",
}

// orderedSFStrings is an ordered list of service flags from highest to
// lowest.
var orderedSFStrings = []ServiceFlag{
	SFNodeNetwork,
	SFNodeBloom,
	SFOpenService,
	SFTxFiltering,
}

// String returns the ServiceFlag in human-readable form.
func (f ServiceFlag) String() string {
	// No flags are set.
	if f == 0 {
		return "0x0"
	}

	// Add individual bit flags.
	s := ""
	for _, flag := range orderedSFStrings {
		if f&flag == flag {
			s += sfStrings[flag] + "|"
			f -= flag
		}
	}

	// Add any remaining flags which aren't accounted for as hex.
	s = strings.TrimRight(s, "|")
	if f != 0 {
		s += "|0x" + strconv.FormatUint(uint64(f), 16)
	}
	s = strings.TrimLeft(s, "|")
	return s
}
