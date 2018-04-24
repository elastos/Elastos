package bloom

import (
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type MerkleProof struct {
	BlockHash    Uint256
	Height       uint32
	Transactions uint32
	Hashes       []*Uint256
	Flags        []byte
}

func (p *MerkleProof) Serialize(w io.Writer) error {
	return WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		uint32(len(p.Hashes)),
		p.Hashes,
		p.Flags,
	)
}

func (p *MerkleProof) Deserialize(r io.Reader) error {
	err := ReadElements(r,
		&p.BlockHash,
		&p.Height,
		&p.Transactions,
	)

	hashes, err := ReadUint32(r)
	if err != nil {
		return err
	}

	p.Hashes = make([]*Uint256, hashes)
	return ReadElements(r, &p.Hashes, &p.Flags)
}
