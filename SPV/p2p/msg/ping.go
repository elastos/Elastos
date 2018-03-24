package msg

import (
	"bytes"
	"SPVWallet/core/serialization"
)

type Ping struct {
	Height uint64
}

func NewPingMsg(height uint32) ([]byte, error) {
	ping := new(Ping)

	ping.Height = uint64(height)

	body, err := ping.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("ping", body)
}

func (p *Ping) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint64(buf, p.Height)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (p *Ping) Deserialize(msg []byte) error {
	var err error
	buf := bytes.NewReader(msg)
	p.Height, err = serialization.ReadUint64(buf)
	return err
}
