package msg

import (
	"bytes"
	"encoding/binary"
	"errors"
)

type Version struct {
	Version   uint32
	Services  uint64
	TimeStamp uint32
	Port      uint16
	Nonce     uint64
	Height    uint64
	Relay     uint8
}

func (msg *Version) CMD() string {
	return "version"
}

func (msg *Version) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, msg)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *Version) Deserialize(body []byte) error {
	buf := bytes.NewBuffer(body)
	err := binary.Read(buf, binary.LittleEndian, msg)
	if err != nil {
		return errors.New("Deserialize version message content error")
	}

	return nil
}
