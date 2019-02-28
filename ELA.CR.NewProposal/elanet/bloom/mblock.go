package bloom

import (
	"github.com/elastos/Elastos.ELA/common"
)

// MBlock is used to house intermediate information needed to generate a
// MerkleBlock according to a filter.
type MBlock struct {
	NumTx       uint32
	AllHashes   []*common.Uint256
	FinalHashes []*common.Uint256
	MatchedBits []byte
	Bits        []byte
}

// calcTreeWidth calculates and returns the the number of nodes (width) or a
// merkle tree at the given depth-first height.
func (m *MBlock) CalcTreeWidth(height uint32) uint32 {
	return (m.NumTx + (1 << height) - 1) >> height
}

// calcHash returns the hash for a sub-tree given a depth-first height and
// node position.
func (m *MBlock) CalcHash(height, pos uint32) *common.Uint256 {
	if height == 0 {
		return m.AllHashes[pos]
	}

	var right *common.Uint256
	left := m.CalcHash(height-1, pos*2)
	if pos*2+1 < m.CalcTreeWidth(height-1) {
		right = m.CalcHash(height-1, pos*2+1)
	} else {
		right = left
	}
	return HashMerkleBranches(left, right)
}

// HashMerkleBranches takes two hashes, treated as the left and right tree
// nodes, and returns the hash of their concatenation.  This is a helper
// function used to aid in the generation of a merkle tree.
func HashMerkleBranches(left *common.Uint256, right *common.Uint256) *common.Uint256 {
	// Concatenate the left and right nodes.
	var hash [common.UINT256SIZE * 2]byte
	copy(hash[:common.UINT256SIZE], left[:])
	copy(hash[common.UINT256SIZE:], right[:])

	newHash := common.Uint256(common.Sha256D(hash[:]))
	return &newHash
}

// traverseAndBuild builds a partial merkle tree using a recursive depth-first
// approach.  As it calculates the hashes, it also saves whether or not each
// node is a parent node and a list of final hashes to be included in the
// merkle block.
func (m *MBlock) TraverseAndBuild(height, pos uint32) {
	// Determine whether this node is a parent of a matched node.
	var isParent byte
	for i := pos << height; i < (pos+1)<<height && i < m.NumTx; i++ {
		isParent |= m.MatchedBits[i]
	}
	m.Bits = append(m.Bits, isParent)

	// When the node is a leaf node or not a parent of a matched node,
	// append the hash to the list that will be part of the final merkle
	// block.
	if height == 0 || isParent == 0x00 {
		m.FinalHashes = append(m.FinalHashes, m.CalcHash(height, pos))
		return
	}

	// At this point, the node is an internal node and it is the parent of
	// of an included leaf node.

	// Descend into the left child and process its sub-tree.
	m.TraverseAndBuild(height-1, pos*2)

	// Descend into the right child and process its sub-tree if
	// there is one.
	if pos*2+1 < m.CalcTreeWidth(height-1) {
		m.TraverseAndBuild(height-1, pos*2+1)
	}
}
