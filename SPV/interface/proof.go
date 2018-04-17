package _interface

import (
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/common/serialize"
)

type Proof struct {
	BlockHash    Uint256
	Height       uint32
	Transactions uint32
	Hashes       []*Uint256
	Flags        []byte
}

func (p *Proof) Serialize(w io.Writer) error {
	return serialize.WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		uint32(len(p.Hashes)),
		p.Hashes,
		p.Flags,
	)
}

func (p *Proof) Deserialize(r io.Reader) error {
	err := serialize.ReadElements(r,
		&p.BlockHash,
		&p.Height,
		&p.Transactions,
	)

	hashes, err := serialize.ReadUint32(r)
	if err != nil {
		return err
	}

	p.Hashes = make([]*Uint256, hashes)
	return serialize.ReadElements(r, &p.Hashes, &p.Flags)
}
