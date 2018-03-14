package p2p

import (
	"bytes"
	"encoding/binary"
	"errors"
	"time"

	"SPVWallet/crypto"
	"SPVWallet/db"
)

type Version struct {
	Header
	VersionContent
}

type VersionContent struct {
	Version      uint32
	Services     uint64
	TimeStamp    uint32
	Port         uint16
	HttpInfoPort uint16
	Cap          [32]byte
	Nonce        uint64
	UserAgent    uint8
	Height       uint64
	Relay        uint8
}

func NewVersionMsg(peer *Peer) ([]byte, error) {
	msg := new(Version)

	// build msg content
	msg.VersionContent = *newVersionContent(peer)

	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, msg.VersionContent)
	if err != nil {
		return nil, err
	}

	// Fake public key
	pk := new(crypto.PublicKey)
	err = pk.Serialize(buf)
	if err != nil {
		return nil, err
	}

	// build msg header
	msg.Header = *BuildHeader("version", buf.Bytes())

	return msg.Serialize()
}

// Only local peer will use this method, so the parameters are fixed
func newVersionContent(peer *Peer) *VersionContent {
	content := new(VersionContent)
	content.Version = peer.Version()
	content.Services = peer.Services()
	content.TimeStamp = uint32(time.Now().UnixNano())
	content.Port = peer.Port()
	content.HttpInfoPort = 0
	content.Cap[0] = 0x00
	content.Nonce = peer.ID()
	content.UserAgent = 0x00
	content.Height = uint64(db.GetBlockchain().Height())
	content.Relay = peer.Relay()
	return content
}

func (v *Version) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, v.Header)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, v.VersionContent)
	if err != nil {
		return nil, err
	}

	// Fake public key
	pk := new(crypto.PublicKey)
	err = pk.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (v *Version) Deserialize(buf []byte) error {
	msg := bytes.NewBuffer(buf)

	err := binary.Read(msg, binary.LittleEndian, &v.Header)
	if err != nil {
		return errors.New("Deserialize version message header error")
	}

	err = binary.Read(msg, binary.LittleEndian, &v.VersionContent)
	if err != nil {
		return errors.New("Deserialize version message content error")
	}

	return nil
}
