package message

import (
	"bytes"
	"encoding/binary"

	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialization"
	"Elastos.ELA/core/ledger"
)

type MerkleBlock struct {
	Header
	BlockHeader  ledger.Blockdata
	Transactions uint32
	Hashes       []*Uint256
	Flags        []byte
}

func (mb *MerkleBlock) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, mb.Header)
	if err != nil {
		return nil, err
	}

	mb.BlockHeader.Serialize(buf)

	err = serialization.WriteUint32(buf, mb.Transactions)
	if err != nil {
		return nil, err
	}

	for _, hash := range mb.Hashes {
		_, err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	err = serialization.WriteVarBytes(buf, mb.Flags)

	return buf.Bytes(), nil
}

func (mb *MerkleBlock) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := binary.Read(buf, binary.LittleEndian, &mb.Header)
	if err != nil {
		return err
	}

	err = mb.BlockHeader.Deserialize(buf)
	if err != nil {
		return err
	}

	mb.Transactions, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	for i := uint32(0); i < mb.Transactions; i++ {
		var txId Uint256
		err := txId.Deserialize(buf)
		if err != nil {
			return err
		}
		mb.Hashes = append(mb.Hashes, &txId)
	}

	mb.Flags, err = serialization.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	return nil
}
