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

func (mb *MerkleBlock) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := mb.BlockHeader.Serialize(buf)
	if err != nil {
		return nil, err
	}

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
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (mb *MerkleBlock) Deserialize(msg []byte) error {
	buf := bytes.NewReader(msg)
	err := mb.BlockHeader.Deserialize(buf)
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
