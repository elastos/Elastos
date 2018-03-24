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

func (inv *Inventory) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint8(buf, inv.Type)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteUint32(buf, inv.Count)
	if err != nil {
		return nil, err
	}

	err = binary.Write(buf, binary.LittleEndian, inv.Data)
	if err != nil {
		return nil, err
	}

	return BuildMessage("inv", buf.Bytes())
}

func (inv *Inventory) Deserialize(msg []byte) error {
	var err error
	buf := bytes.NewReader(msg)
	inv.Type, err = serialization.ReadUint8(buf)
	if err != nil {
		return err
	}

	inv.Count, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	inv.Data = make([]byte, inv.Count*UINT256SIZE)
	err = binary.Read(buf, binary.LittleEndian, &inv.Data)
	if err != nil {
		return err
	}

	return nil
}
