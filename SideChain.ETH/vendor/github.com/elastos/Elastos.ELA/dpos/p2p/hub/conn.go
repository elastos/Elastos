/*
Conn is a wrapper of the origin network connection.  It resolves the handshake
information from the first version message including magic number, PID, network
address etc.
*/
package hub

import (
	"bytes"
	"io"
	"net"

	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Conn is a wrapper of the origin network connection.
type Conn struct {
	net.Conn // The origin network connection.
	buf      *bytes.Buffer
	magic    uint32
	pid      [33]byte
	target   [16]byte
	addr     net.Addr
}

// Magic returns the magic number resolved from message header.
func (c *Conn) Magic() uint32 {
	return c.magic
}

// PID returns the PID resolved from the version message.  It represents who
// is connecting.
func (c *Conn) PID() [33]byte {
	return c.pid
}

// Target returns the Target PID resolved from the version message.  It used
// when a service behind the hub want to connect to another service,
// representing who the service is going to connect.
func (c *Conn) Target() [16]byte {
	return c.target
}

// NetAddr returns the network address resolve from the origin connection and
// the version message.
func (c *Conn) NetAddr() net.Addr {
	return c.addr
}

// Read warps the origin Read method without knowing we have intercepted the
// version message.
func (c *Conn) Read(b []byte) (n int, err error) {
	n, err = c.buf.Read(b)
	if n > 0 {
		return n, err
	}
	return c.Conn.Read(b)
}

// WrapConn warps the origin network connection and returns a hub connection
// with the handshake information resolved from version message.
func WrapConn(c net.Conn) (conn *Conn, err error) {
	// Read message header
	var headerBytes [p2p.HeaderSize]byte
	if _, err = io.ReadFull(c, headerBytes[:]); err != nil {
		return
	}

	// Deserialize message header
	var hdr p2p.Header
	if err = hdr.Deserialize(headerBytes[:]); err != nil {
		return
	}

	if hdr.GetCMD() != p2p.CmdVersion {
		return
	}

	// Read payload
	payload := make([]byte, hdr.Length)
	if _, err = io.ReadFull(c, payload[:]); err != nil {
		return
	}

	// Verify checksum
	if err = hdr.Verify(payload); err != nil {
		return
	}

	v := &msg.Version{}
	err = v.Deserialize(bytes.NewReader(payload))
	if err != nil {
		return
	}

	buf := bytes.NewBuffer(headerBytes[:])
	buf.Write(payload)

	conn = &Conn{
		Conn:   c,
		buf:    buf,
		magic:  hdr.Magic,
		pid:    v.PID,
		target: v.Target,
		addr:   newNetAddr(c.RemoteAddr(), v.Port),
	}
	return
}

// newNetAddr creates a net.Addr with the origin net.Addr and port.
func newNetAddr(addr net.Addr, port uint16) net.Addr {
	// addr will be a net.TCPAddr when not using a proxy.
	if tcpAddr, ok := addr.(*net.TCPAddr); ok {
		return &net.TCPAddr{IP: tcpAddr.IP, Port: int(port)}
	}

	// For the most part, addr should be one of the two above cases, but
	// to be safe, fall back to trying to parse the information from the
	// address string as a last resort.
	host, _, err := net.SplitHostPort(addr.String())
	if err != nil {
		return nil
	}
	return &net.TCPAddr{IP: net.ParseIP(host), Port: int(port)}
}
