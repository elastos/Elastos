package sdk

import (
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// The protocol version implemented SPV protocol
	ProtocolVersion = p2p.EIP001Version

	// OpenService is a flag used to indicate a peer provides open service.
	OpenService = 1 << 2
)
