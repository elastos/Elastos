package p2p

import (
	"time"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
)

type Message interface {
	CMD() string
	Serialize() ([]byte, error)
	Deserialize(msg []byte) error
}

// Only local peer will use this method, so the parameters are fixed
func NewVersion() *Version {
	peer := pm.Local()
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

func (pm *PeerManager) DecodeMessageBuf(peer *Peer, buf []byte) {
	if len(buf) < HEADERLEN {
		log.Error("Message length is not enough")
		return
	}

	msg, err := pm.makeMessage(buf)
	if err != nil {
		log.Error("Make message error, ", err)
		return
	}

	err = msg.Deserialize(buf[HEADERLEN:])
	if err != nil {
		log.Error("Deserialize message ", msg.CMD(), " error: ", err)
		return
	}

	pm.handleMessage(peer, msg)
}

func (pm *PeerManager) parseHeader(buf []byte) (*Header, error) {
	hdr := new(Header)
	err := hdr.Deserialize(buf)
	if err = hdr.Verify(buf[HEADERLEN:]); err != nil {
		return nil, err
	}
	return hdr, nil
}

func (pm *PeerManager) makeMessage(buf []byte) (Message, error) {
	hdr, err := pm.parseHeader(buf)
	if err != nil {
		return nil, err
	}

	log.Debug("Receive message: ", hdr.GetCMD())
	var msg Message

	cmd := hdr.GetCMD()
	switch cmd {
	case "version":
		msg = new(Version)
	case "verack":
		msg = new(VerAck)
	case "getaddr":
		msg = new(AddrsReq)
	case "addr":
		msg = new(Addrs)
	default:
		return pm.OnMakeMessage(cmd)
	}

	return msg, nil
}

func (pm *PeerManager) handleMessage(peer *Peer, msg Message) {
	var err error
	switch msg := msg.(type) {
	case *Version:
		err = pm.OnVersion(peer, msg)
	case *VerAck:
		err = pm.OnVerAck(peer, msg)
	case *AddrsReq:
		err = pm.OnAddrsReq(peer, msg)
	case *Addrs:
		err = pm.OnAddrs(peer, msg)
	default:
		err = pm.OnHandleMessage(peer, msg)
	}

	if err != nil {
		log.Error("Handle message error,", err)
	}
}
