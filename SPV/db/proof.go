package db

import (
	. "github.com/elastos/Elastos.ELA.SPV/core"
	"github.com/elastos/Elastos.ELA.SPV/core/serialization"
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
	return serialization.WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		uint32(len(p.Hashes)),
		p.Hashes,
		p.Flags,
	)
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
	return serialization.ReadElements(r, &p.Hashes, &p.Flags)
}
