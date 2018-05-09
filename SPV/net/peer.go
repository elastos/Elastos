package net

import (
	"fmt"
	"net"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type PeerHandler interface {
	MessageHandler
	OnDisconnected(peer *Peer)
}

type Peer struct {
	id         uint64
	version    uint32
	services   uint64
	ip16       [16]byte
	port       uint16
	lastActive time.Time
	height     uint64
	relay      uint8 // 1 for true 0 for false

	p2p.PeerState
	conn    net.Conn
	handler PeerHandler

	msgHelper *p2p.MsgHelper
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

func (peer *Peer) Addr() *msg.Addr {
	return msg.NewPeerAddr(peer.services, peer.ip16, peer.port, peer.id)
}

func (peer *Peer) Relay() uint8 {
	return peer.relay
}

func (peer *Peer) SetRelay(relay uint8) {
	peer.relay = relay
}

func (peer *Peer) Disconnect() {
	if peer.State() != p2p.INACTIVITY {
		peer.SetState(p2p.INACTIVITY)
		peer.conn.Close()
	}
}

func (peer *Peer) SetInfo(msg *msg.Version) {
	peer.id = msg.Nonce
	peer.port = msg.Port
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
	case p2p.ErrDisconnected:
		peer.handler.OnDisconnected(peer)
	case p2p.ErrUnmatchedMagic:
		log.Error("Decode message error:", p2p.ErrUnmatchedMagic)
		peer.Disconnect()
	default:
		log.Error(err, ", peer id is: ", peer.ID())
	}
}

func (peer *Peer) OnMakeMessage(cmd string) (p2p.Message, error) {
	return peer.handler.MakeMessage(cmd)
}

func (peer *Peer) OnMessageDecoded(msg p2p.Message) {
	peer.handler.HandleMessage(peer, msg)
}

func (peer *Peer) Read() {
	peer.msgHelper.Read()
}

func (peer *Peer) Send(msg p2p.Message) {
	if peer.State() == p2p.INACTIVITY {
		return
	}

	buf, err := peer.msgHelper.Build(msg)
	if err != nil {
		log.Error("Serialize message failed, ", err)
		return
	}

	_, err = peer.conn.Write(buf)
	if err != nil {
		log.Error("Error sending message to peer ", err)
		peer.handler.OnDisconnected(peer)
	}
}

func (peer *Peer) NewVersionMsg() *msg.Version {
	version := new(msg.Version)
	version.Version = peer.Version()
	version.Services = peer.Services()
	version.TimeStamp = uint32(time.Now().UnixNano())
	version.Port = peer.Port()
	version.Nonce = peer.ID()
	version.Height = peer.Height()
	version.Relay = peer.Relay()
	return version
}
