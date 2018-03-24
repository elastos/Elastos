package msg

import (
	"bytes"
	"encoding/binary"
	"errors"
)

type Version struct {
	Version      uint32
	Services     uint64
	TimeStamp    uint32
	Port         uint16
	Nonce        uint64
	Height       uint64
	Relay        uint8
}

func NewVersionMsg(data Version) ([]byte, error) {
	body, err := data.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("version", body)
}

func (v *Version) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, v)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (v *Version) Deserialize(buf []byte) error {
	msg := bytes.NewBuffer(buf)
	err := binary.Read(msg, binary.LittleEndian, v)
	if err != nil {
		return errors.New("Deserialize version message content error")
	}

	return nil
}
