package server

import (
	"net"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	defaultDataDir               = "./data"
	defaultMaxPeers              = 125
	defaultBanThreshold   uint32 = 100
	defaultBanDuration           = time.Hour * 24
	defaultConnectTimeout        = time.Second * 30
)

// Config is a descriptor which specifies the server instance configuration.
type Config struct {
	// DataDir is the data path to store peer addresses etc.
	DataDir string

	// MagicNumber is the peer-to-peer network ID to connect to.
	MagicNumber uint32

	// ProtocolVersion represent the protocol version you are supporting.
	ProtocolVersion uint32

	// Services represent which services you are supporting.
	Services uint64

	// SeedPeers are the peers to connect with at startup.
	SeedPeers []string

	// ListenAddrs are the addresses listen on to accept peer connections.
	ListenAddrs []string

	// ExternalIPs are a list of local addresses we claim to listen on to peers.
	ExternalIPs []string

	// Use UPnP to map our listening port outside of NAT
	Upnp bool

	// DefaultPort defines the default peer-to-peer port for the network.
	DefaultPort uint16

	// Disable listening for incoming connections.
	DisableListen bool

	// DisableRelayTx specifies if the remote peer should be informed to
	// not send inv messages for transactions.
	DisableRelayTx bool

	// Max number of inbound and outbound peers.
	MaxPeers int

	// Disable banning of misbehaving peers.
	DisableBanning bool

	// Maximum allowed ban score before disconnecting and banning misbehaving
	// peers.
	BanThreshold uint32

	// How long to ban misbehaving peers.  Valid time units are {s, m, h}.
	// Minimum 1 second
	BanDuration time.Duration

	// IP networks or IPs that will not be banned. (eg. 192.168.1.0/24 or ::1)
	Whitelists []*net.IPNet

	// TargetOutbound is the number of outbound network connections to maintain.
	// Defaults to 8.
	TargetOutbound int

	// OnNewPeer will be invoked when a new peer connected.
	OnNewPeer func(IPeer)

	// OnDonePeer will be invoked when a peer disconnected.
	OnDonePeer func(IPeer)

	// MakeEmptyMessage will be invoked to creates a message of the appropriate
	// concrete type based on the command.
	MakeEmptyMessage func(string) (p2p.Message, error)

	// BestHeight will be invoked to get current best height.
	BestHeight func() uint64

	// PingNonce will be invoked to get a nonce when sending a ping message.
	PingNonce func() uint64

	// PongNonce will be invoked to get a nonce when sending a ping message.
	PongNonce func() uint64
}

func (cfg *Config) normalize() {
	defaultPort := strconv.FormatUint(uint64(cfg.DefaultPort), 10)

	// Add default port to all seed peer addresses if needed and remove
	// duplicate addresses.
	cfg.SeedPeers = normalizeAddresses(cfg.SeedPeers, defaultPort)

	// Add default port to all listener addresses if needed and remove
	// duplicate addresses.
	cfg.ListenAddrs = normalizeAddresses(cfg.ListenAddrs, defaultPort)
}

// inWhitelist returns whether the IP address is included in the whitelisted
// networks and IPs.
func (cfg *Config) inWhitelist(addr net.Addr) bool {
	if len(cfg.Whitelists) == 0 {
		return false
	}

	host, _, err := net.SplitHostPort(addr.String())
	if err != nil {
		log.Warnf("Unable to SplitHostPort on '%s': %v", addr, err)
		return false
	}
	ip := net.ParseIP(host)
	if ip == nil {
		log.Warnf("Unable to parse IP '%s'", addr)
		return false
	}

	for _, ipnet := range cfg.Whitelists {
		if ipnet.Contains(ip) {
			return true
		}
	}
	return false
}

func dialTimeout(addr net.Addr) (net.Conn, error) {
	return net.DialTimeout(addr.Network(), addr.String(), defaultConnectTimeout)
}

// removeDuplicateAddresses returns a new slice with all duplicate entries in
// addrs removed.
func removeDuplicateAddresses(addrs []string) []string {
	result := make([]string, 0, len(addrs))
	seen := map[string]struct{}{}
	for _, val := range addrs {
		if _, ok := seen[val]; !ok {
			result = append(result, val)
			seen[val] = struct{}{}
		}
	}
	return result
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

// normalizeAddresses returns a new slice with all the passed peer addresses
// normalized with the given default port, and all duplicates removed.
func normalizeAddresses(addrs []string, defaultPort string) []string {
	for i, addr := range addrs {
		addrs[i] = normalizeAddress(addr, defaultPort)
	}

	return removeDuplicateAddresses(addrs)
}

// NewDefaultConfig returns a new config instance filled by default settings
// for the server.
func NewDefaultConfig(
	magic, pver uint32,
	services uint64,
	defaultPort uint16,
	seeds, listenAddrs []string,
	onNewPeer func(IPeer),
	onDonePeer func(IPeer),
	makeEmptyMessage func(string) (p2p.Message, error),
	bestHeight func() uint64) *Config {
	return &Config{
		MagicNumber:      magic,
		ProtocolVersion:  pver,
		Services:         services,
		SeedPeers:        seeds,
		ListenAddrs:      listenAddrs,
		ExternalIPs:      nil,
		Upnp:             false,
		DefaultPort:      defaultPort,
		DisableListen:    false,
		DisableRelayTx:   false,
		MaxPeers:         defaultMaxPeers,
		DisableBanning:   false,
		BanThreshold:     defaultBanThreshold,
		BanDuration:      defaultBanDuration,
		Whitelists:       nil,
		TargetOutbound:   defaultTargetOutbound,
		OnNewPeer:        onNewPeer,
		OnDonePeer:       onDonePeer,
		MakeEmptyMessage: makeEmptyMessage,
		BestHeight:       bestHeight,
		PingNonce:        bestHeight,
		PongNonce:        bestHeight,
	}
}
