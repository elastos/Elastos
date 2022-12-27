// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package crypto

import (
	"errors"

	"github.com/elastos/Elastos.ELA/common"
)

type MerkleTree struct {
	Depth uint
	Root  *MerkleTreeNode
}

type MerkleTreeNode struct {
	Hash  common.Uint256
	Left  *MerkleTreeNode
	Right *MerkleTreeNode
}

func (t *MerkleTreeNode) IsLeaf() bool {
	return t.Left == nil && t.Right == nil
}

//use []Uint256 to create a new MerkleTree
func NewMerkleTree(hashes []common.Uint256) (*MerkleTree, error) {
	if len(hashes) == 0 {
		return nil, errors.New("NewMerkleTree input no item error.")
	}

	var height uint = 1
	nodes := generateLeaves(hashes)
	for len(nodes) > 1 {
		nodes = levelUp(nodes)
		height += 1
	}
	mt := &MerkleTree{
		Root:  nodes[0],
		Depth: height,
	}
	return mt, nil

}

//Generate the leaves nodes
func generateLeaves(hashes []common.Uint256) []*MerkleTreeNode {
	var leaves []*MerkleTreeNode
	for _, d := range hashes {
		node := &MerkleTreeNode{
			Hash: d,
		}
		leaves = append(leaves, node)
	}
	return leaves
}

//calc the next level's hash use double sha256
func levelUp(nodes []*MerkleTreeNode) []*MerkleTreeNode {
	var nextLevel []*MerkleTreeNode
	for i := 0; i < len(nodes)/2; i++ {
		node := &MerkleTreeNode{
			Hash:  ComputeParent(nodes[i*2].Hash, nodes[i*2+1].Hash),
			Left:  nodes[i*2],
			Right: nodes[i*2+1],
		}
		nextLevel = append(nextLevel, node)
	}
	if len(nodes)%2 == 1 {
		node := &MerkleTreeNode{
			Hash:  ComputeParent(nodes[len(nodes)-1].Hash, nodes[len(nodes)-1].Hash),
			Left:  nodes[len(nodes)-1],
			Right: nodes[len(nodes)-1],
		}
		nextLevel = append(nextLevel, node)
	}
	return nextLevel
}

//input a []uint256, create a MerkleTree & calc the root hash
func ComputeRoot(hashes []common.Uint256) (common.Uint256, error) {
	if len(hashes) == 0 {
		return common.Uint256{}, errors.New("NewMerkleTree input no item error.")
	}
	if len(hashes) == 1 {
		return hashes[0], nil
	}
	tree, _ := NewMerkleTree(hashes)
	return tree.Root.Hash, nil
}

func ComputeParent(left common.Uint256, right common.Uint256) common.Uint256 {
	// Concatenate the left and right nodes
	var sha [64]byte
	copy(sha[:32], left[:])
	copy(sha[32:], right[:])
	return common.Hash(sha[:])
}
