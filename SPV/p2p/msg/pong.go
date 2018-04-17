package msg

import (
	"bytes"

	"encoding/binary"
)

type Pong struct {
	Ping
}

func (msg *Pong) CMD() string {
	return "pong"
}

func (msg *Pong) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, msg.Height)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
