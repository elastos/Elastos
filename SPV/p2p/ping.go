package p2p

import (
	"bytes"
	"SPVWallet/core/serialization"
)

type Ping struct {
	Height uint64
}

func NewPing(height uint32) *Ping {
	ping := new(Ping)
	ping.Height = uint64(height)
	return ping
}

func (msg *Ping) CMD() string {
	return "ping"
}

func (msg *Ping) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint64(buf, msg.Height)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (msg *Ping) Deserialize(body []byte) error {
	var err error
	buf := bytes.NewReader(body)
	msg.Height, err = serialization.ReadUint64(buf)
	return err
}
