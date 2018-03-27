package db

import (
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
	"io"
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

func (p *Proof) Serialize(w io.Writer) error {
	err := serialization.WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		p.Hashes,
		p.Flags,
	)
	if err != nil {
		return err
	}

	return nil
}

func (p *Proof) Deserialize(r io.Reader) error {
	err := p.BlockHash.Deserialize(r)
	if err != nil {
		return err
	}

	p.Height, err = serialization.ReadUint32(r)
	if err != nil {
		return err
	}

	p.Transactions, err = serialization.ReadUint32(r)
	if err != nil {
		return err
	}

	for i := uint32(0); i < p.Transactions; i++ {
		var txId Uint256
		err := txId.Deserialize(r)
		if err != nil {
			return err
		}
		p.Hashes = append(p.Hashes, &txId)
	}

	p.Flags, err = serialization.ReadVarBytes(r)
	if err != nil {
		return err
	}

	return nil
}
