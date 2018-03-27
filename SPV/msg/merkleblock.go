package msg

import (
	"bytes"
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
	"SPVWallet/db"
)

type MerkleBlock struct {
	BlockHeader db.Header
	db.Proof
}

func (msg *MerkleBlock) CMD() string {
	return "merkleblock"
}

func (msg *MerkleBlock) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := msg.BlockHeader.Serialize(buf)
	if err != nil {
		return nil, err
	}

	err = serialization.WriteElements(buf, msg.Transactions, uint32(len(msg.Hashes)), msg.Hashes, msg.Flags)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *MerkleBlock) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := msg.BlockHeader.Deserialize(buf)
	if err != nil {
		return err
	}

	msg.Transactions, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	hashes, err := serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	msg.Hashes = make([]*Uint256, hashes)
	err = serialization.ReadElements(buf, &msg.Hashes, &msg.Flags)
	if err != nil {
		return err
	}

	return nil
}
