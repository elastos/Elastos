// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package pact

import (
	"strconv"
	"strings"
)

// Release version numbers
const (
	// ProtocolVersion is the latest protocol version this package supports.
	ProtocolVersion = DPOSStartVersion

	// DPOSStartVersion is the protocol version which switch to DPOS protocol.
	DPOSStartVersion uint32 = 20000

	// CRProposalVersion is the protocol version which switch to CR proposal protocol.
	CRProposalVersion uint32 = 30000

	// EBIP001Version is the protocol version starts to support SPV protocol.
	EBIP001Version uint32 = 10001

	// MaxBlocksPerMsg is the maximum number of blocks allowed per message.
	MaxBlocksPerMsg = 500

	// MaxTxPoolSize is the maximum size of txs allowed in transaction pool.
	MaxTxPoolSize = 20000000
)

var (
	// MaxBlockContextSize is the maximum number of bytes allowed per block context.
	MaxBlockContextSize uint32 = 8000000

	// MaxBlockHeaderSize is the maximum number of bytes allowed per block header.
	MaxBlockHeaderSize uint32 = 1000000

	// MaxTxPerBlock is the maximux number of transactions allowed per block.
	MaxTxPerBlock uint32 = 10000
)

// ServiceFlag identifies services supported by a peer.
type ServiceFlag uint64

const (
	// SFNodeNetwork is a flag used to indicate a peer is a full node.
	SFNodeNetwork ServiceFlag = 1 << iota

	// SFTxFiltering is a flag used to indicate a peer supports transaction
	// filtering.
	SFTxFiltering

	// SFNodeBloom is a flag used to indicate a peer supports bloom filtering.
	SFNodeBloom
)

// Map of service flags back to their constant names for pretty printing.
var sfStrings = map[ServiceFlag]string{
	SFNodeNetwork: "SFNodeNetwork",
	SFTxFiltering: "SFTxFiltering",
	SFNodeBloom:   "SFNodeBloom",
}

// orderedSFStrings is an ordered list of service flags from highest to
// lowest.
var orderedSFStrings = []ServiceFlag{
	SFNodeNetwork,
	SFTxFiltering,
	SFNodeBloom,
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
