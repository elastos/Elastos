package p2p

import (
	"errors"
	"time"

	. "SPVWallet/p2p/msg"
	"SPVWallet/log"
)

var listeners *Listeners

type Listeners struct {
	OnVersion     func(peer *Peer, msg *Version) error
	OnVerAck      func(peer *Peer, msg *VerAck) error
	OnAddrs       func(peer *Peer, msg *Addrs) error
	OnAddrsReq    func(peer *Peer, msg *AddrsReq) error
	OnPing        func(peer *Peer, msg *Ping) error
	OnPong        func(peer *Peer, msg *Pong) error
	OnInventory   func(peer *Peer, msg *Inventory) error
	OnMerkleBlock func(peer *Peer, msg *MerkleBlock) error
	OnTxn         func(peer *Peer, msg *Txn) error
	OnNotFound    func(peer *Peer, msg *NotFound) error
	OnDisconnect  func(peer *Peer)
}

type Message interface {
	Serialize() ([]byte, error)
	Deserialize(msg []byte) error
}

func SetListeners(ls *Listeners) {
	listeners = ls
}

// Only local peer will use this method, so the parameters are fixed
func NewVersion() Version {
	peer := LocalPeer()
	content := new(Version)
	content.Version = peer.Version()
	content.Services = peer.Services()
	content.TimeStamp = uint32(time.Now().UnixNano())
	content.Port = peer.Port()
	content.Nonce = peer.ID()
	content.Height = peer.Height()
	content.Relay = peer.Relay()
	return *content
}

func HandleMessage(peer *Peer, buf []byte) {
	if len(buf) < HEADERLEN {
		log.Error("Message length is not enough")
		return
	}

	msg, err := makeMessage(buf)
	if err != nil {
		log.Error("Make message error,", err)
		return
	}

	err = msg.Deserialize(buf[HEADERLEN:])
	if err != nil {
		log.Error("Deserialize message err:", err)
		return
	}

	allocateMessage(peer, msg)
}

func parseHeader(buf []byte) (*Header, error) {
	hdr := new(Header)
	err := hdr.Deserialize(buf)
	if err = hdr.Verify(buf[HEADERLEN:]); err != nil {
		return nil, err
	}
	return hdr, nil
}

func makeMessage(buf []byte) (Message, error) {
	hdr, err := parseHeader(buf)
	if err != nil {
		return nil, err
	}

	log.Info("Receive message:", hdr.GetCMD())
	var msg Message

	switch hdr.GetCMD() {
	case "version":
		msg = new(Version)
	case "verack":
		msg = new(VerAck)
	case "ping":
		msg = new(Ping)
	case "pong":
		msg = new(Pong)
	case "getaddr":
		msg = new(AddrsReq)
	case "addr":
		msg = new(Addrs)
	case "inv":
		msg = new(Inventory)
	case "tx":
		msg = new(Txn)
	case "merkleblock":
		msg = new(MerkleBlock)
	case "notfound":
		msg = new(NotFound)
	default:
		return nil, errors.New("Received unsupported message, CMD " + hdr.GetCMD())
	}

	return msg, nil
}

func allocateMessage(peer *Peer, msg Message) {
	var err error
	switch msg.(type) {
	case *Version:
		err = listeners.OnVersion(peer, msg.(*Version))
	case *VerAck:
		err = listeners.OnVerAck(peer, msg.(*VerAck))
	case *Ping:
		err = listeners.OnPing(peer, msg.(*Ping))
	case *Pong:
		err = listeners.OnPong(peer, msg.(*Pong))
	case *AddrsReq:
		err = listeners.OnAddrsReq(peer, msg.(*AddrsReq))
	case *Addrs:
		err = listeners.OnAddrs(peer, msg.(*Addrs))
	case *Inventory:
		err = listeners.OnInventory(peer, msg.(*Inventory))
	case *MerkleBlock:
		err = listeners.OnMerkleBlock(peer, msg.(*MerkleBlock))
	case *Txn:
		err = listeners.OnTxn(peer, msg.(*Txn))
	case *NotFound:
		err = listeners.OnNotFound(peer, msg.(*NotFound))
	}

	if err != nil {
		log.Error("Allocate message error,", err)
	}
}
