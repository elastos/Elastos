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

func (p *Proof) Serialize(w io.Writer) error {
	err := serialization.WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		uint32(len(p.Hashes)),
		p.Hashes,
		p.Flags,
	)
	if err != nil {
		return err
	}

	return nil
}

func (p *Proof) Deserialize(r io.Reader) error {
	err := serialization.ReadElements(r,
		&p.BlockHash,
		&p.Height,
		&p.Transactions,
	)

	hashes, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}

	p.Hashes = make([]*Uint256, hashes)
	err = serialization.ReadElements(r, &p.Hashes, &p.Flags)
	if err != nil {
		return err
	}

	return nil
}
