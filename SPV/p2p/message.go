package p2p

import (
	"errors"
	"time"

	. "SPVWallet/msg"
	"SPVWallet/log"
)

var callback func(peer *Peer, msg Message)

type Message interface {
	CMD() string
	Serialize() ([]byte, error)
	Deserialize(msg []byte) error
}

func RegisterCallback(msgCallback func(peer *Peer, msg Message)) {
	callback = msgCallback
}

// Only local peer will use this method, so the parameters are fixed
func NewVersion() *Version {
	peer := LocalPeer()
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

	callback(peer, msg)
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
