package msg

import (
	"bytes"
	"encoding/binary"
	"errors"
)

type Version struct {
	Header
	VersionData
}

type VersionData struct {
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

func NewVersionMsg(data VersionData) ([]byte, error) {
	msg := new(Version)

	// build msg content
	msg.VersionData = data

	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("version", body)
}

func (v *Version) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, v.VersionData)
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

	err = binary.Read(msg, binary.LittleEndian, &v.VersionData)
	if err != nil {
		return errors.New("Deserialize version message content error")
	}

	return nil
}
