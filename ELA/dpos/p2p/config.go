package p2p

import (
	"net"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	defaultConnectTimeout = time.Second * 30
)

// Config is a descriptor which specifies the server instance configuration.
type Config struct {
	// PID is the public key id of this server.
	PID [32]byte

	// MagicNumber is the peer-to-peer network ID to connect to.
	MagicNumber uint32

	// ProtocolVersion represent the protocol version you are supporting.
	ProtocolVersion uint32

	// Services represent which services you are supporting.
	Services uint64

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// MakeEmptyMessage will be invoked to creates a message of the appropriate
	// concrete type based on the command.
	MakeEmptyMessage func(string) (p2p.Message, error)
}

func dialTimeout(addr net.Addr) (net.Conn, error) {
	return net.DialTimeout(addr.Network(), addr.String(), defaultConnectTimeout)
}

// normalizeAddress returns addr with the passed default port appended if
// there is not already a port specified.
func normalizeAddress(addr, defaultPort string) string {
	_, _, err := net.SplitHostPort(addr)
	if err != nil {
		return net.JoinHostPort(addr, defaultPort)
	}
	return addr
}
