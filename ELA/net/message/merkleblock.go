package message

import (
	"Elastos.ELA/bloom"
	"Elastos.ELA/core/ledger"
)

type MerkleBlockMsg struct {
	Header
	MerkleBlock bloom.MerkleBlock
}

// NewMerkleBlock returns a new *MerkleBlock
func NewMerkleBlockMsg(block *ledger.Block, filter *bloom.Filter) ([]byte, error) {
	// Create and return the merkle block.
	merkleBlock := bloom.NewMerkleBlock(block, filter)

	body, err := merkleBlock.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("merkleblock", body)
}
