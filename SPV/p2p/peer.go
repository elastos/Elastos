package p2p

import (
	"fmt"
	"io"
	"net"
	"strings"
	"sync"
	"time"

	"SPVWallet/config"
	. "SPVWallet/p2p/msg"
)

const (
	PeerVersion = 1 // The min p2p protocol version to support spv
	SPVPeerPort = 20866
	ServiceSPV  = 1 << 2
	MaxBufLen   = 1024 * 16
)

// Peer states
const (
	INIT       = iota
	HAND
	HANDSHAKE
	HANDSHAKED
	ESTABLISH
	INACTIVITY
)

type PeerState struct {
	sync.RWMutex
	state int
}

func (ps *PeerState) SetState(state int) {
	ps.Lock()
	defer ps.Unlock()

	ps.state = state
}

func (ps *PeerState) State() int {
	ps.RLock()
	defer ps.RUnlock()

	return ps.state
}

type Peer struct {
	// info
	id         uint64
	version    uint32
	services   uint64
	port       uint16
	lastActive time.Time
	height     uint64
	relay      uint8 // 1 for true 0 for false

	PeerState
	conn net.Conn

	msgBuf MsgBuf
}

type MsgBuf struct {
	buf []byte
	len int
}

func (buf *MsgBuf) Append(msg []byte) {
	buf.buf = append(buf.buf, msg...)
}

func (buf *MsgBuf) Buf() []byte {
	return buf.buf
}

func (buf *MsgBuf) Reset() {
	buf.buf = nil
	buf.len = 0
}

func NewPeer(conn net.Conn) *Peer {
	return &Peer{
		conn: conn,
	}
}

func (peer *Peer) ID() uint64 {
	return peer.id
}

func (peer *Peer) Version() uint32 {
	return peer.version
}

func (peer *Peer) Services() uint64 {
	return peer.services
}

func (peer *Peer) IP16() [16]byte {
	fullAddr := peer.conn.RemoteAddr().String()
	portIndex := strings.LastIndex(fullAddr, ":")
	ip := net.ParseIP(string([]byte(fullAddr)[:portIndex])).To16()

	ip16 := [16]byte{}
	copy(ip16[:], ip[:])

	return ip16
}

func (peer *Peer) Port() uint16 {
	return peer.port
}

func (peer *Peer) LastActive() time.Time {
	return peer.lastActive
}

func (peer *Peer) Addr() *PeerAddr {
	return NewPeerAddr(peer.services, peer.IP16(), peer.port, peer.id)
}

func (peer *Peer) Relay() uint8 {
	return peer.relay
}

func (peer *Peer) Disconnect() {
	peer.SetState(INACTIVITY)
	peer.conn.Close()
}

func (peer *Peer) Update(msg *Version) {
	peer.id = msg.Nonce
	peer.version = msg.Version
	peer.services = msg.Services
	peer.port = msg.Port
	peer.lastActive = time.Now()
	peer.height = msg.Height
	peer.relay = msg.Relay
}

func (peer *Peer) SetHeight(height uint64) {
	peer.height = height
}

func (peer *Peer) Height() uint64 {
	return peer.height
}

func (peer *Peer) Read() {
	buf := make([]byte, MaxBufLen)
	for {
		len, err := peer.conn.Read(buf[0:MaxBufLen-1])
		buf[MaxBufLen-1] = 0 //Prevent overflow
		switch err {
		case nil:
			peer.lastActive = time.Now()
			peer.unpackMessage(buf[:len])
		case io.EOF:
			fmt.Errorf("Read peer io.EOF: %s, peer id is %d ", err.Error(), peer.ID())
			goto DISCONNECT
		default:
			fmt.Errorf("Read peer connection error ", err.Error())
			goto DISCONNECT
		}
	}

DISCONNECT:
	listeners.OnDisconnect(peer)
}

func (peer *Peer) unpackMessage(buf []byte) {

	if len(buf) == 0 {
		return
	}

	if peer.msgBuf.len == 0 { // Buffering message header
		index := HEADERLEN - len(peer.msgBuf.Buf())
		if index > len(buf) { // header not finished, continue read
			index = len(buf)
			peer.msgBuf.Append(buf[0:index])
			return
		}

		peer.msgBuf.Append(buf[0:index])

		var header Header
		err := header.Deserialize(peer.msgBuf.Buf())
		if err != nil {
			fmt.Println("Get error message header, relocate the msg header")
			peer.msgBuf.Reset()
			return
		}

		if header.Magic != config.Config().Magic {
			fmt.Println("Magic not match, disconnect peer")
			listeners.OnDisconnect(peer)
			return
		}

		peer.msgBuf.len = int(header.Length)
		buf = buf[index:]
	}

	msgLen := peer.msgBuf.len

	if len(buf) == msgLen { // Just read the full message

		peer.msgBuf.Append(buf[:])
		go HandleMessage(peer, peer.msgBuf.Buf())
		peer.msgBuf.Reset()

	} else if len(buf) < msgLen { // Read part of the message

		peer.msgBuf.Append(buf[:])
		peer.msgBuf.len = msgLen - len(buf)

	} else { // Read more than the message

		peer.msgBuf.Append(buf[0:msgLen])
		go HandleMessage(peer, peer.msgBuf.Buf())
		peer.msgBuf.Reset()
		peer.unpackMessage(buf[msgLen:])
	}
}

func (peer *Peer) Send(msg []byte) {
	if peer.State() == INACTIVITY {
		return
	}

	_, err := peer.conn.Write(msg)
	if err != nil {
		fmt.Errorf("Error sending message to peer ", err)
		listeners.OnDisconnect(peer)
	}
}
