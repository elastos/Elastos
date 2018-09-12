package bloom

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

type MerkleProof struct {
	BlockHash    common.Uint256
	Height       uint32
	Transactions uint32
	Hashes       []*common.Uint256
	Flags        []byte
}

func (p *MerkleProof) Serialize(w io.Writer) error {
	return common.WriteElements(w,
		p.BlockHash,
		p.Height,
		p.Transactions,
		uint32(len(p.Hashes)),
		p.Hashes,
		p.Flags,
	)
}

func (p *MerkleProof) Deserialize(r io.Reader) error {
	err := common.ReadElements(r,
		&p.BlockHash,
		&p.Height,
		&p.Transactions,
	)

	count, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	if count > msg.MaxTxPerBlock {
		return fmt.Errorf("MerkleProof.Deserialize too many transaction"+
			" hashes for message [count %v, max %v]", count, msg.MaxTxPerBlock)
	}

	p.Hashes = make([]*common.Uint256, count)
	return common.ReadElements(r, &p.Hashes, &p.Flags)
}
