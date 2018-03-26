package db

import (
	"bytes"

	. "SPVWallet/core"
	"SPVWallet/core/serialization"
)

type Proof struct {
	BlockHash    Uint256
	Height       uint32
	Transactions uint32
	Hashes       []*Uint256
	Flags        []byte
}

func NewProof(blockHash Uint256, height, transactions uint32, hashes []*Uint256, flags []byte) *Proof {
	return &Proof{
		BlockHash:    blockHash,
		Height:       height,
		Transactions: transactions,
		Hashes:       hashes,
		Flags:        flags,
	}
}

func (p *Proof) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteElements(buf,
		p.BlockHash,
		p.Height,
		p.Transactions,
		p.Hashes,
		p.Flags,
	)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (mb *Proof) Deserialize(buf []byte) error {
	reader := bytes.NewReader(buf)
	err := mb.BlockHash.Deserialize(reader)
	if err != nil {
		return err
	}

	mb.Height, err = serialization.ReadUint32(reader)
	if err != nil {
		return err
	}

	mb.Transactions, err = serialization.ReadUint32(reader)
	if err != nil {
		return err
	}

	for i := uint32(0); i < mb.Transactions; i++ {
		var txId Uint256
		err := txId.Deserialize(reader)
		if err != nil {
			return err
		}
		mb.Hashes = append(mb.Hashes, &txId)
	}

	mb.Flags, err = serialization.ReadVarBytes(reader)
	if err != nil {
		return err
	}

	return nil
}
