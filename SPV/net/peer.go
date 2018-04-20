package net

import (
	"fmt"
	"net"
	"strings"
	"strconv"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"

	. "github.com/elastos/Elastos.ELA.Utility/p2p"
	. "github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

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
	conn net.Conn

	reader *MsgReader
}

func (peer *Peer) String() string {
	return fmt.Sprint("\nPeer: {",
		"\n\tID:", peer.id,
		"\n\tVersion:", peer.version,
		"\n\tServices:", peer.services,
		"\n\tPort:", peer.port,
		"\n\tLastActive:", peer.lastActive,
		"\n\tHeight:", peer.height,
		"\n\tRelay:", peer.relay,
		"\n\tState:", peer.PeerState.String(),
		"\n\tAddr:", peer.Addr().String(),
		"\n}")
}

func NewPeer(conn net.Conn) *Peer {
	peer := new(Peer)
	peer.conn = conn
	peer.ip16, peer.port = addrFromConn(conn)
	peer.reader = NewMsgReader(conn, peer)
	return peer
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
	if peer.State() != INACTIVITY {
		peer.SetState(INACTIVITY)
		peer.conn.Close()
	}
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

func (peer *Peer) OnDecodeError(err error) {
	switch err {
	case ErrDisconnected:
		pm.DisconnectPeer(peer)
	case ErrUnmatchedMagic:
		log.Error("Decode message error:", ErrUnmatchedMagic)
		peer.Disconnect()
	default:
		log.Error(err, ", peer id is: ", peer.ID())
	}
}

func (peer *Peer) OnMakeMessage(cmd string) (Message, error) {
	return pm.makeMessage(cmd)
}

func (peer *Peer) OnMessageDecoded(msg Message) {
	pm.handleMessage(peer, msg)
}

func (peer *Peer) Read() {
	peer.reader.Read()
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
		pm.DisconnectPeer(peer)
	}
}

func (peer *Peer) NewVersionMsg() *Version {
	version := new(Version)
	version.Version = peer.Version()
	version.Services = peer.Services()
	version.TimeStamp = uint32(time.Now().UnixNano())
	version.Port = peer.Port()
	version.Nonce = peer.ID()
	version.Height = peer.Height()
	version.Relay = peer.Relay()
	return version
}
