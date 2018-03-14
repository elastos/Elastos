package p2p

import (
	"bytes"
	"encoding/binary"

	"SPVWallet/core/serialization"
	"SPVWallet/db"
)

type Ping struct {
	Header
	Height uint64
}

func NewPingMsg() ([]byte, error) {
	ping := new(Ping)

	ping.Height = uint64(db.GetBlockchain().Height())

	buf := new(bytes.Buffer)
	serialization.WriteUint64(buf, ping.Height)

	ping.Header = *BuildHeader("ping", buf.Bytes())

	return ping.Serialize()
}

func (p *Ping) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, p.Header)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint64(buf, p.Height)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (p *Ping) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &p.Header)
	if err != nil {
		return err
	}

	p.Height, err = serialization.ReadUint64(buf)
	return err
}
