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

	err = serialization.WriteUint32(buf, msg.Transactions)
	if err != nil {
		return nil, err
	}

	for _, hash := range msg.Hashes {
		_, err := hash.Serialize(buf)
		if err != nil {
			return nil, err
		}
	}

	err = serialization.WriteVarBytes(buf, msg.Flags)
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

	for i := uint32(0); i < msg.Transactions; i++ {
		var txId Uint256
		err := txId.Deserialize(buf)
		if err != nil {
			return err
		}
		msg.Hashes = append(msg.Hashes, &txId)
	}

	msg.Flags, err = serialization.ReadVarBytes(buf)
	if err != nil {
		return err
	}

	return nil
}
