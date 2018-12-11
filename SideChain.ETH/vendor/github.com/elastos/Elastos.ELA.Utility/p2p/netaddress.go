package p2p

import (
	"fmt"
	"io"
	"net"
	"time"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

// NetAddress defines information about a peer on the network including the time
// it was last seen, the services it supports, its IP address, and port.
type NetAddress struct {
	// Last time the address was seen.  This is, unfortunately, encoded as a
	// uint32 on the wire and therefore is limited to 2106.  This field is
	// not present in the version message (Version) nor was it
	// added until protocol version >= NetAddressTimeVersion.
	Timestamp time.Time

	// Bitfield which identifies the services supported by the address.
	Services uint64

	// IP address of the peer.
	IP net.IP

	// Port the peer is using. This is encoded in little endian.
	Port uint16
}

func (na NetAddress) String() string {
	return fmt.Sprint(na.IP.String(), ":", na.Port)
}

// HasService returns whether the specified service is supported by the address.
func (na *NetAddress) HasService(service uint64) bool {
	return na.Services&service == service
}

// AddService adds service as a supported service by the peer generating the
// message.
func (na *NetAddress) AddService(service uint64) {
	na.Services |= service
}

// Serialize serializes a NetAddress to w.
func (na *NetAddress) Serialize(w io.Writer) error {
	// Ensure to always write 16 bytes even if the ip is nil.
	var ip [16]byte
	if na.IP != nil {
		copy(ip[:], na.IP.To16())
	}

	return common.WriteElements(w, na.Timestamp.Unix(), na.Services, ip, na.Port)
}

// readNetAddress reads an encoded NetAddress from r.
func (na *NetAddress) Deserialize(r io.Reader) error {
	var timestamp int64
	var ip [16]byte

	if err := common.ReadElements(r, &timestamp, &na.Services, &ip, &na.Port); err != nil {
		return err
	}

	na.Timestamp = time.Unix(timestamp, 0)
	na.IP = net.IP(ip[:])
	return nil
}

// NewNetAddressIPPort returns a new NetAddress using the provided IP, port, and
// supported services with defaults for the remaining fields.
func NewNetAddressIPPort(ip net.IP, port uint16, services uint64) *NetAddress {
	return NewNetAddressTimestamp(time.Now(), services, ip, port)
}

// NewNetAddressTimestamp returns a new NetAddress using the provided
// timestamp, IP, port, and supported services. The timestamp is rounded to
// single second precision.
func NewNetAddressTimestamp(
	timestamp time.Time, services uint64, ip net.IP, port uint16) *NetAddress {
	// Limit the timestamp to one second precision since the protocol
	// doesn't support better.
	na := NetAddress{
		Timestamp: time.Unix(timestamp.Unix(), 0),
		Services:  services,
		IP:        ip,
		Port:      port,
	}
	return &na
}

// NewNetAddress returns a new NetAddress using the provided TCP address and
// supported services with defaults for the remaining fields.
func NewNetAddress(addr *net.TCPAddr, services uint64) *NetAddress {
	return NewNetAddressIPPort(addr.IP, uint16(addr.Port), services)
}
