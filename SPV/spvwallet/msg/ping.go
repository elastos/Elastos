package msg

import (
	"bytes"
	"encoding/binary"
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
	err := binary.Write(buf, binary.LittleEndian, msg.Height)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func (msg *Ping) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	return binary.Read(buf, binary.LittleEndian, &msg.Height)
}
