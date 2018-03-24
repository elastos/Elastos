package msg

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

func (p *Ping) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint64(buf, p.Height)
	if err != nil {
		return nil, err
	}
	return BuildMessage("ping", buf.Bytes())
}

func (p *Ping) Deserialize(msg []byte) error {
	var err error
	buf := bytes.NewReader(msg)
	p.Height, err = serialization.ReadUint64(buf)
	return err
}
