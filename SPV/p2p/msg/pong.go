package msg

import (
	"bytes"

	"SPVWallet/core/serialization"
)

type Pong struct {
	Ping
}

func NewPong(height uint32) *Pong {
	pong := new(Pong)
	pong.Height = uint64(height)
	return pong
}

func (p *Pong) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint64(buf, p.Height)
	if err != nil {
		return nil, err
	}
	return BuildMessage("pong", buf.Bytes())
}
