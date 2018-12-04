package net

import (
	"fmt"
	"net"
	"time"
	"sync/atomic"
	"strings"

	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type PeerHandler struct {
	OnDisconnected func(peer *Peer)
	MakeMessage    func(cmd string) (p2p.Message, error)
	HandleMessage  func(peer *Peer, msg p2p.Message) error
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

	state   int32
	conn    net.Conn
	handler PeerHandler

	msgHelper *p2p.MsgHelper
}

func (p *Peer) String() string {
	var state p2p.PeerState
	state.SetState(uint(p.state))
	return fmt.Sprint(
		"ID:", p.id,
		", Version:", p.version,
		", Services:", p.services,
		", Port:", p.port,
		", LastActive:", p.lastActive,
		", Height:", p.height,
		", Relay:", p.relay,
		", State:", state.String(),
		", Addr:", p.Addr().String())
}

func (p *Peer) ID() uint64 {
	return p.id
}

func (p *Peer) SetID(id uint64) {
	p.id = id
}

func (p *Peer) Version() uint32 {
	return p.version
}

func (p *Peer) SetVersion(version uint32) {
	p.version = version
}

func (p *Peer) Services() uint64 {
	return p.services
}

func (p *Peer) SetServices(services uint64) {
	p.services = services
}

func (p *Peer) IP16() [16]byte {
	return p.ip16
}

func (p *Peer) Port() uint16 {
	return p.port
}

func (p *Peer) SetPort(port uint16) {
	p.port = port
}

func (p *Peer) LastActive() time.Time {
	return p.lastActive
}

func (p *Peer) Addr() *p2p.NetAddress {
	return p2p.NewNetAddress(p.services, p.ip16, p.port, p.id)
}

func (p *Peer) Relay() uint8 {
	return p.relay
}

func (p *Peer) SetRelay(relay uint8) {
	p.relay = relay
}

func (p *Peer) State() int32 {
	return atomic.LoadInt32(&p.state)
}

func (p *Peer) SetState(state int32) {
	atomic.StoreInt32(&p.state, state)
}

func (p *Peer) Disconnect() {
	// Return if peer already disconnected
	if p.State() == p2p.INACTIVITY {
		return
	}
	p.SetState(p2p.INACTIVITY)
	p.conn.Close()
}

func (p *Peer) SetInfo(msg *msg.Version) {
	p.id = msg.Nonce
	p.port = msg.Port
	p.version = msg.Version
	p.services = msg.Services
	p.lastActive = time.Now()
	p.height = msg.Height
	p.relay = msg.Relay
}

func (p *Peer) SetHeight(height uint64) {
	p.height = height
}

func (p *Peer) Height() uint64 {
	return p.height
}

func (p *Peer) OnError(err error) {
	switch err {
	case p2p.ErrInvalidHeader,
		p2p.ErrUnmatchedMagic,
		p2p.ErrMsgSizeExceeded:
		log.Error(err)
		p.Disconnect()
	case p2p.ErrDisconnected:
		p.handler.OnDisconnected(p)
	default:
		log.Error(err, ", peer id is: ", p.ID())
	}
}

func (p *Peer) OnMakeMessage(cmd string) (p2p.Message, error) {
	if p.State() == p2p.INACTIVITY {
		return nil, fmt.Errorf("-----> [%s] from INACTIVE peer [%d]", cmd, p.id)
	}
	p.lastActive = time.Now()
	return p.handler.MakeMessage(cmd)
}

func (p *Peer) OnMessageDecoded(message p2p.Message) {
	log.Debugf("-----> [%s] from peer [%d] STARTED", message.CMD(), p.id)
	if err := p.handler.HandleMessage(p, message); err != nil {
		log.Error(err)
	}
	log.Debugf("-----> [%s] from peer [%d] FINISHED", message.CMD(), p.id)
}

func (p *Peer) Start() {
	p.msgHelper.Read()
}

func (p *Peer) Send(msg p2p.Message) {
	if p.State() == p2p.INACTIVITY {
		log.Errorf("-----> Push [%s] to INACTIVE peer [%d]", msg.CMD(), p.id)
		return
	}
	log.Debugf("-----> Push [%s] to peer [%d] STARTED", msg.CMD(), p.id)
	p.msgHelper.Write(msg)
	log.Debugf("-----> Push [%s] to peer [%d] FINISHED", msg.CMD(), p.id)
}

func (p *Peer) NewVersionMsg() *msg.Version {
	version := new(msg.Version)
	version.Version = p.Version()
	version.Services = p.Services()
	version.TimeStamp = uint32(time.Now().UnixNano())
	version.Port = p.Port()
	version.Nonce = p.ID()
	version.Height = p.Height()
	version.Relay = p.Relay()
	return version
}

func (p *Peer) SetPeerHandler(handler PeerHandler) {
	p.handler = handler
}

func NewPeer(magic, maxMsgSize uint32, conn net.Conn) *Peer {
	peer := new(Peer)
	peer.conn = conn
	copy(peer.ip16[:], getIp(conn))
	peer.msgHelper = p2p.NewMsgHelper(magic, maxMsgSize, conn, peer)
	return peer
}

func getIp(conn net.Conn) []byte {
	addr := conn.RemoteAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	return net.ParseIP(string([]byte(addr)[:portIndex])).To16()
}
