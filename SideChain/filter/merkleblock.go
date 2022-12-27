package filter

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

// mBlock is used to house intermediate information needed to generate a
// MerkleBlock according to a filter.
type mBlock struct {
	NumTx       uint32
	AllHashes   []*common.Uint256
	FinalHashes []*common.Uint256
	MatchedBits []byte
	Bits        []byte
}

// calcTreeWidth calculates and returns the the number of nodes (width) or a
// merkle tree at the given depth-first height.
func (m *mBlock) calcTreeWidth(height uint32) uint32 {
	return (m.NumTx + (1 << height) - 1) >> height
}

// calcHash returns the hash for a sub-tree given a depth-first height and
// node position.
func (m *mBlock) calcHash(height, pos uint32) *common.Uint256 {
	if height == 0 {
		return m.AllHashes[pos]
	}

	var right *common.Uint256
	left := m.calcHash(height-1, pos*2)
	if pos*2+1 < m.calcTreeWidth(height-1) {
		right = m.calcHash(height-1, pos*2+1)
	} else {
		right = left
	}
	return hashMerkleBranches(left, right)
}

// hashMerkleBranches takes two hashes, treated as the left and right tree
// nodes, and returns the hash of their concatenation.  This is a helper
// function used to aid in the generation of a merkle tree.
func hashMerkleBranches(left *common.Uint256, right *common.Uint256) *common.Uint256 {
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
func (m *mBlock) traverseAndBuild(height, pos uint32) {
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
		m.FinalHashes = append(m.FinalHashes, m.calcHash(height, pos))
		return
	}

	// At this point, the node is an internal node and it is the parent of
	// of an included leaf node.

	// Descend into the left child and process its sub-tree.
	m.traverseAndBuild(height-1, pos*2)

	// Descend into the right child and process its sub-tree if
	// there is one.
	if pos*2+1 < m.calcTreeWidth(height-1) {
		m.traverseAndBuild(height-1, pos*2+1)
	}
}

// NewMerkleBlock returns a new *MerkleBlock
func NewMerkleBlock(block *types.Block, filter *Filter) (*msg.MerkleBlock, []uint32) {
	NumTx := uint32(len(block.Transactions))
	mBlock := mBlock{
		NumTx:       NumTx,
		AllHashes:   make([]*common.Uint256, 0, NumTx),
		MatchedBits: make([]byte, 0, NumTx),
	}

	// Find and keep track of any transactions that match the filter.
	var matchedIndexes []uint32
	for index, tx := range block.Transactions {
		if filter.Match(tx) {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x01)
			matchedIndexes = append(matchedIndexes, uint32(index))
		} else {
			mBlock.MatchedBits = append(mBlock.MatchedBits, 0x00)
		}
		txHash := tx.Hash()
		mBlock.AllHashes = append(mBlock.AllHashes, &txHash)
	}

	// Calculate the number of merkle branches (height) in the tree.
	height := uint32(0)
	for mBlock.calcTreeWidth(height) > 1 {
		height++
	}

	// Build the depth-first partial merkle tree.
	mBlock.traverseAndBuild(height, 0)

	// Create and return the merkle block.
	merkleBlock := &msg.MerkleBlock{
		Header:       block.Header,
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

	return merkleBlock, matchedIndexes
}

type merkleNode struct {
	p uint32          // position in the binary tree
	h *common.Uint256 // hash
}

func (node merkleNode) String() string {
	return fmt.Sprint("Node{pos:", node.p, ", hash:", node.h, "}")
}

// given n merkle leaves, how deep is the tree?
// iterate shifting left until greater than n
func treeDepth(n uint32) (e uint32) {
	for ; (1 << e) < n; e++ {
	}
	return
}

// smallest power of 2 that can contain n
func nextPowerOfTwo(n uint32) uint32 {
	return 1 << treeDepth(n) // 2^exponent
}

// check if a node is populated based on node position and size of tree
func inDeadZone(pos, size uint32) bool {
	msb := nextPowerOfTwo(size)
	last := size - 1      // last valid position is 1 less than size
	if pos > (msb<<1)-2 { // greater than root; not even in the tree
		return true
	}
	h := msb
	for pos >= h {
		h = h>>1 | msb
		last = last>>1 | msb
	}
	return pos > last
}

// take in a merkle block, parse through it, and return txids indicated
// If there's any problem return an error.  Checks self-consistency only.
// doing it with a stack instead of recursion.  Because...
// OK I don't know why I'm just not in to recursion OK?
func CheckMerkleBlock(m msg.MerkleBlock) ([]*common.Uint256, error) {
	if m.Transactions == 0 {
		return nil, fmt.Errorf("No transactions in merkleblock")
	}
	if len(m.Flags) == 0 {
		return nil, fmt.Errorf("No flag bits")
	}
	var header = m.Header.(*types.Header)
	var s []merkleNode      // the stack
	var r []*common.Uint256 // slice to return; txids we care about

	// set initial position to root of merkle tree
	msb := nextPowerOfTwo(m.Transactions) // most significant bit possible
	pos := (msb << 1) - 2                 // current position in tree

	var i uint8 // position in the current flag byte
	var tip int
	// main loop
	for {
		tip = len(s) - 1 // slice position of stack tip
		// First check if stack operations can be performed
		// is stack one filled item?  that's complete.
		if tip == 0 && s[0].h != nil {
			if s[0].h.IsEqual(header.Base.MerkleRoot) {
				return r, nil
			}
			return nil, fmt.Errorf("computed root %s but expect %s\n",
				s[0].h.String(), header.Base.MerkleRoot.String())
		}
		// is current position in the tree's dead zone? partial parent
		if inDeadZone(pos, m.Transactions) {
			// create merkle parent from single side (left)
			h, err := makeMerkleParent(s[tip].h, nil)
			if err != nil {
				return r, err
			}
			s[tip-1].h = h
			s = s[:tip]          // remove 1 from stack
			pos = s[tip-1].p | 1 // move position to parent's sibling
			continue
		}
		// does stack have 3+ items? and are last 2 items filled?
		if tip > 1 && s[tip-1].h != nil && s[tip].h != nil {
			//fmt.Printf("nodes %d and %d combine into %d\n",
			//	s[tip-1].p, s[tip].p, s[tip-2].p)
			// combine two filled nodes into parent node
			h, err := makeMerkleParent(s[tip-1].h, s[tip].h)
			if err != nil {
				return r, err
			}
			s[tip-2].h = h
			// remove children
			s = s[:tip-1]
			// move position to parent's sibling
			pos = s[tip-2].p | 1
			continue
		}

		// no stack ops to perform, so make new node from message hashes
		if len(m.Hashes) == 0 {
			return nil, fmt.Errorf("Ran out of hashes at position %d.", pos)
		}
		if len(m.Flags) == 0 {
			return nil, fmt.Errorf("Ran out of flag bits.")
		}
		var n merkleNode // make new node
		n.p = pos        // set current position for new node

		if pos&msb != 0 { // upper non-txid hash
			if m.Flags[0]&(1<<i) == 0 { // flag bit says fill node
				n.h = m.Hashes[0]       // copy hash from message
				m.Hashes = m.Hashes[1:] // pop off message
				if pos&1 != 0 {         // right side; ascend
					pos = pos>>1 | msb
				} else { // left side, go to sibling
					pos |= 1
				}
			} else { // flag bit says skip; put empty on stack and descend
				pos = (pos ^ msb) << 1 // descend to left
			}
			s = append(s, n) // push new node on stack
		} else { // bottom row txid; flag bit indicates tx of interest
			if pos >= m.Transactions {
				// this can't happen because we check deadzone above...
				return nil, fmt.Errorf("got into an invalid txid node")
			}
			n.h = m.Hashes[0]           // copy hash from message
			m.Hashes = m.Hashes[1:]     // pop off message
			if m.Flags[0]&(1<<i) != 0 { //txid of interest
				r = append(r, n.h)
			}
			if pos&1 == 0 { // left side, go to sibling
				pos |= 1
			} // if on right side we don't move; stack ops will move next
			s = append(s, n) // push new node onto the stack
		}

		// done with pushing onto stack; advance flag bit
		i++
		if i == 8 { // move to next byte
			i = 0
			m.Flags = m.Flags[1:]
		}
	}
	return nil, fmt.Errorf("ran out of things to do?")
}

func makeMerkleParent(left *common.Uint256, right *common.Uint256) (*common.Uint256, error) {
	// dupes can screw things up; CVE-2012-2459. check for them
	if left != nil && right != nil && left.IsEqual(*right) {
		return nil, errors.New("DUP HASH CRASH")
	}
	// if left child is nil, output nil.  Need this for hard mode.
	if left == nil {
		return nil, errors.New("Left child is nil")
	}
	// if right is nil, hash left with itself
	if right == nil {
		right = left
	}

	// Concatenate the left and right nodes
	var sha [64]byte
	copy(sha[:32], left[:])
	copy(sha[32:], right[:])

	parent := common.Uint256(common.Sha256D(sha[:]))
	return &parent, nil
}
