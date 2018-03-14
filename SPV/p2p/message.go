package p2p

import (
	"errors"
	"fmt"

)

var listeners *Listeners

type Message interface {
	Serialize() ([]byte, error)
	Deserialize(msg []byte) error
}

func HandleMessage(peer *Peer, buf []byte) error {
	if len(buf) < HEADERLEN {
		return errors.New("Message length is not enough")
	}

	msg, err := makeMessage(buf)
	if err != nil {
		return err
	}

	err = msg.Deserialize(buf)
	if err != nil {
		return errors.New(fmt.Sprint("Deserialize message err:", err))
	}

	return allocateMessage(peer, msg)
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

func allocateMessage(peer *Peer, msg Message) error {
	switch msg.(type) {
	case *Version:
		return listeners.OnVersion(peer, msg.(*Version))
	case *VerAck:
		return listeners.OnVerAck(peer, msg.(*VerAck))
	case *Ping:
		return listeners.OnPing(peer, msg.(*Ping))
	case *Pong:
		return listeners.OnPong(peer, msg.(*Pong))
	case *AddrsReq:
		return listeners.OnAddrsReq(peer, msg.(*AddrsReq))
	case *Addrs:
		return listeners.OnAddrs(peer, msg.(*Addrs))
	case *Inventory:
		return listeners.OnInventory(peer, msg.(*Inventory))
	case *MerkleBlock:
		return listeners.OnMerkleBlock(peer, msg.(*MerkleBlock))
	case *Txn:
		return listeners.OnTxn(peer, msg.(*Txn))
	case *NotFound:
		return listeners.OnNotFound(peer, msg.(*NotFound))
	}
	return nil
}
