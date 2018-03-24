package msg

import (
	"bytes"
	"encoding/binary"
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
)

const (
	TRANSACTION = 0x01
	BLOCK       = 0x02
)

type Inventory struct {
	Type  uint8
	Count uint32
	Data  []byte
}

func (msg *Inventory) CMD() string {
	return "inv"
}

func (msg *Inventory) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint8(buf, msg.Type)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint32(buf, msg.Count)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, msg.Data)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *Inventory) Deserialize(body []byte) error {
	var err error
	buf := bytes.NewReader(body)
	msg.Type, err = serialization.ReadUint8(buf)
	if err != nil {
		return err
	}

	msg.Count, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	msg.Data = make([]byte, msg.Count*UINT256SIZE)
	err = binary.Read(buf, binary.LittleEndian, &msg.Data)
	if err != nil {
		return err
	}

	return nil
}
