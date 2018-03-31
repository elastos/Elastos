package p2p

import (
	"fmt"
	"io"
	"net"
	"strings"
	"strconv"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
)

const (
	ProtocolVersion = 1 // The min p2p protocol version to support spv
	MaxBufLen       = 1024 * 16
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

func (ps *PeerState) String() string {
	switch ps.state {
	case INIT:
		return "INIT"
	case HAND:
		return "HAND"
	case HANDSHAKE:
		return "HANDSHAKE"
	case HANDSHAKED:
		return "HANDSHAKED"
	case ESTABLISH:
		return "ESTABLISH"
	case INACTIVITY:
		return "INACTIVITY"
	}
	return "Unknown peer state"
}

type Peer struct {
	// info
	id         uint64
	version    uint32
	services   uint64
	ip16       [16]byte
	port       uint16
	lastActive time.Time
	height     uint64
	relay      uint8 // 1 for true 0 for false

	PeerState
	conn         net.Conn
	OnDisconnect func(*Peer)

	msgBuf MsgBuf
}

func (peer *Peer) String() string {
	return "\nPeer: {" +
		"\n\tID:" + fmt.Sprint(peer.id) +
		"\n\tVersion:" + fmt.Sprint(peer.version) +
		"\n\tServices:" + fmt.Sprint(peer.services) +
		"\n\tPort:" + fmt.Sprint(peer.port) +
		"\n\tLastActive:" + fmt.Sprint(peer.lastActive) +
		"\n\tHeight:" + fmt.Sprint(peer.height) +
		"\n\tRelay:" + fmt.Sprint(peer.relay) +
		"\n\tState:" + peer.PeerState.String() +
		"\n\tAddr:" + peer.Addr().String() +
		"\n}"
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
	ip16, port := addrFromConn(conn)
	return &Peer{
		conn: conn,
		ip16: ip16,
		port: port,
	}
}

func addrFromConn(conn net.Conn) ([16]byte, uint16) {
	addr := conn.RemoteAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	port, _ := strconv.ParseUint(string([]byte(addr)[portIndex+1:]), 10, 16)
	ip := net.ParseIP(string([]byte(addr)[:portIndex])).To16()

	ip16 := [16]byte{}
	copy(ip16[:], ip[:])

	return ip16, uint16(port)
}

func (peer *Peer) ID() uint64 {
	return peer.id
}

func (peer *Peer) SetID(id uint64) {
	peer.id = id
}

func (peer *Peer) Version() uint32 {
	return peer.version
}

func (peer *Peer) SetVersion(version uint32) {
	peer.version = version
}

func (peer *Peer) Services() uint64 {
	return peer.services
}

func (peer *Peer) SetServices(servcies uint64) {
	peer.services = servcies
}

func (peer *Peer) IP16() [16]byte {
	return peer.ip16
}

func (peer *Peer) Port() uint16 {
	return peer.port
}

func (peer *Peer) SetPort(port uint16) {
	peer.port = port
}

func (peer *Peer) LastActive() time.Time {
	return peer.lastActive
}

func (peer *Peer) Addr() *Addr {
	return NewPeerAddr(peer.services, peer.ip16, peer.port, peer.id)
}

func (peer *Peer) Relay() uint8 {
	return peer.relay
}

func (peer *Peer) SetRelay(relay uint8) {
	peer.relay = relay
}

func (peer *Peer) Disconnect() {
	peer.SetState(INACTIVITY)
	peer.conn.Close()
}

func (peer *Peer) SetInfo(msg *Version) {
	peer.id = msg.Nonce
	peer.version = msg.Version
	peer.services = msg.Services
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
			log.Error("Read peer io.EOF:", err, ", peer id is: ", peer.ID())
			goto DISCONNECT
		default:
			log.Error("Read peer connection error: ", err.Error())
			goto DISCONNECT
		}
	}

DISCONNECT:
	log.Trace("Peer IO error, disconnect peer,", peer)
	peer.OnDisconnect(peer)
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

		if header.Magic != Magic {
			log.Error("Magic not match, disconnect peer")
			peer.OnDisconnect(peer)
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

func (peer *Peer) Send(msg Message) {
	if peer.State() == INACTIVITY {
		return
	}

	buf, err := BuildMessage(msg)
	if err != nil {
		log.Error("Serialize message failed, ", err)
		return
	}

	_, err = peer.conn.Write(buf)
	if err != nil {
		log.Error("Error sending message to peer ", err)
		peer.OnDisconnect(peer)
	}
}
