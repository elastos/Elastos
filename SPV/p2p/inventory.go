package p2p

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
	Header
	Type  uint8
	Count uint32
	Data  []byte
}

func NewBlockHashesInventoryMsg(blockHashes []*Uint256) ([]byte, error) {
	msg := new(Inventory)
	msg.Type = BLOCK
	msg.Count = uint32(len(blockHashes))

	data := new(bytes.Buffer)
	for _, hash := range blockHashes {
		_, err := hash.Serialize(data)
		if err != nil {
			return nil, err
		}
	}
	msg.Data = data.Bytes()

	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("inv", body)
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

	err = serialization.WriteVarBytes(buf, inv.Data)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (inv *Inventory) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &inv.Header)
	if err != nil {
		return err
	}

	inv.Type, err = serialization.ReadUint8(buf)
	if err != nil {
		return err
	}

	inv.Count, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	inv.Data, err = serialization.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	return nil
}
