package bloom

import (
	"crypto/rand"
	"fmt"
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/p2p/msg"

	"github.com/elastos/Elastos.ELA.SPV/util"
)

// Ensure header implement BlockHeader interface.
var _ util.BlockHeader = (*header)(nil)

type header struct {
	*types.Header
}

func (h *header) Previous() common.Uint256 {
	return h.Header.Previous
}

func (h *header) Bits() uint32 {
	return h.Header.Bits
}

func (h *header) MerkleRoot() common.Uint256 {
	return h.Header.MerkleRoot
}

func (h *header) PowHash() common.Uint256 {
	return h.AuxPow.ParBlockHeader.Hash()
}

func TestMerkleBlock_GetTxMerkleBranch(t *testing.T) {
	for txs := uint32(1); txs < 300; txs++ {
		run(txs)
	}
}

func run(txs uint32) {
	mBlock := mBlock{
		NumTx:       txs,
		AllHashes:   make([]*common.Uint256, 0, txs),
		MatchedBits: make([]byte, 0, txs),
	}

	matches := randMatches(txs)
	for i := uint32(0); i < txs; i++ {
		if matches[i] {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x01)
		} else {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x00)
		}
		mBlock.AllHashes = append(mBlock.AllHashes, randHash())
	}

	// Calculate the number of merkle branches (height) in the tree.
	height := uint32(0)
	for mBlock.calcTreeWidth(height) > 1 {
		height++
	}

	// Build the depth-first partial merkle tree.
	mBlock.traverseAndBuild(height, 0)

	merkleRoot := *mBlock.calcHash(treeDepth(txs), 0)
	// Create and return the merkle block.
	merkleBlock := msg.MerkleBlock{
		Header: &header{&types.Header{
			MerkleRoot: merkleRoot,
		}},
		Transactions: mBlock.NumTx,
		Hashes:       make([]*common.Uint256, 0, len(mBlock.FinalHashes)),
		Flags:        make([]byte, (len(mBlock.Bits)+7)/8),
	}
	for _, hash := range mBlock.FinalHashes {
		merkleBlock.Hashes = append(merkleBlock.Hashes, hash)
	}
	for i := uint32(0); i < uint32(len(mBlock.Bits)); i++ {
		merkleBlock.Flags[i/8] |= mBlock.Bits[i] << (i % 8)
	}

	txIds, err := CheckMerkleBlock(merkleBlock)
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(0)
	}

	for i := range txIds {
		mb, err := GetTxMerkleBranch(merkleBlock, txIds[i])
		if err != nil {
			fmt.Println(err.Error())
			os.Exit(0)
		}

		calcRoot := auxpow.GetMerkleRoot(*txIds[i], mb.Branches, mb.Index)
		if merkleRoot == calcRoot {
		} else {
			fmt.Printf("Merkle root not match, expect %s result %s", merkleRoot.String(), calcRoot.String())
			os.Exit(0)
		}
	}
}

func randHash() *common.Uint256 {
	var hash common.Uint256
	rand.Read(hash[:])
	return &hash
}

func randMatches(hashes uint32) map[uint32]bool {
	matches := make(map[uint32]bool)
	b := make([]byte, 1)
	for i := uint32(0); i < hashes; i++ {
		rand.Read(b)
		matches[i] = b[0]%2 == 1
	}
	return matches
}
