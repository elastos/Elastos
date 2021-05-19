// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package bloom

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type MerkleBranch struct {
	Branches []common.Uint256
	Index    int
}

func GetTxMerkleBranch(msg msg.MerkleBlock, txID *common.Uint256) (*MerkleBranch, error) {
	mNodes := &merkleNodes{
		root:     msg.Header.(*types.Header).MerkleRoot,
		numTxs:   msg.Transactions,
		allNodes: make(map[uint32]merkleNode),
	}

	mNodes.SetHashes(msg.Hashes)
	mNodes.SetBits(msg.Flags)

	return mNodes.GetMerkleBranch(txID)
}

type merkleNodes struct {
	root     common.Uint256
	numTxs   uint32
	hashes   []*common.Uint256
	bits     []byte
	txIndex  uint32
	route    []uint32
	allNodes map[uint32]merkleNode
}

func (m *merkleNodes) GetMerkleBranch(txID *common.Uint256) (mb *MerkleBranch, err error) {
	m.allNodes, err = m.getNodes()
	if err != nil {
		return nil, err
	}

	m.calcTxIndex(txID)
	m.calcBranchRoute()

	mb = new(MerkleBranch)
	mb.Branches = make([]common.Uint256, 0, len(m.route))
	for i, index := range m.route {
		mb.Branches = append(mb.Branches, *m.allNodes[index].h)
		if index%2 == 0 {
			mb.Index += 1 << uint32(i)
		}
	}

	return mb, nil
}

func (m *merkleNodes) SetHashes(hashes []*common.Uint256) {
	m.hashes = make([]*common.Uint256, 0, len(hashes))
	for _, hash := range hashes {
		m.hashes = append(m.hashes, hash)
	}
}

func (m *merkleNodes) SetBits(flags []byte) {
	m.bits = make([]byte, uint32(len(flags))*8)
	for i := uint32(0); i < uint32(len(flags))*8; i++ {
		m.bits[i] = (flags[i/8] >> (i % 8)) & 1
	}
}

func (m merkleNodes) getNodes() (map[uint32]merkleNode, error) {
	if m.numTxs == 0 {
		return nil, fmt.Errorf("No transactions in merkleblock")
	}
	if len(m.bits) == 0 {
		return nil, fmt.Errorf("No flag bits")
	}
	var s []merkleNode                  // the stack
	var r = make(map[uint32]merkleNode) // the return nodes
	// set initial position to root of merkle tree
	msb := nextPowerOfTwo(m.numTxs) // most significant bit possible
	pos := (msb << 1) - 2           // current position in tree

	var tip int
	// main loop
	for {
		tip = len(s) - 1 // slice position of stack tip
		// First check if stack operations can be performed
		// is stack one filled item?  that's complete.
		if tip == 0 && s[0].h != nil {
			if s[0].h.IsEqual(m.root) {
				return r, nil
			}
			return nil, fmt.Errorf("computed root %s but expect %s\n",
				s[0].h.String(), m.root.String())
		}
		// is current position in the tree's dead zone? partial parent
		if inDeadZone(pos, m.numTxs) {
			// create merkle parent from single side (left)
			h, err := MakeMerkleParent(s[tip].h, nil)
			if err != nil {
				return r, err
			}
			s[tip-1].h = h
			s = s[:tip]          // remove 1 from stack
			pos = s[tip-1].p | 1 // move position to parent's sibling
			r[s[tip-1].p] = s[tip-1]
			continue
		}
		// does stack have 3+ items? and are last 2 items filled?
		if tip > 1 && s[tip-1].h != nil && s[tip].h != nil {
			// combine two filled nodes into parent node
			h, err := MakeMerkleParent(s[tip-1].h, s[tip].h)
			if err != nil {
				return r, err
			}
			s[tip-2].h = h
			// remove children
			s = s[:tip-1]
			// move position to parent's sibling
			pos = s[tip-2].p | 1
			r[s[tip-2].p] = s[tip-2]
			continue
		}

		// no stack ops to perform, so make new node from message hashes
		if len(m.hashes) == 0 {
			return nil, fmt.Errorf("Ran out of hashes at position %d.", pos)
		}
		if len(m.bits) == 0 {
			return nil, fmt.Errorf("Ran out of bits.")
		}
		var n merkleNode // make new node
		n.p = pos        // set current position for new node

		if pos&msb != 0 { // upper non-txid hash
			if m.bits[0] == 0 { // flag bit says fill node
				n.h = m.hashes[0]       // copy hash from message
				m.hashes = m.hashes[1:] // pop off message
				if pos&1 != 0 {         // right side; ascend
					pos = pos>>1 | msb
				} else { // left side, go to sibling
					pos |= 1
				}
				r[n.p] = n
			} else { // flag bit says skip; put empty on stack and descend
				pos = (pos ^ msb) << 1 // descend to left
			}
			s = append(s, n) // push new node on stack
		} else { // bottom row txid; flag bit indicates tx of interest
			if pos >= m.numTxs {
				// this can't happen because we check deadzone above...
				return nil, fmt.Errorf("got into an invalid txid node")
			}
			n.h = m.hashes[0]       // copy hash from message
			m.hashes = m.hashes[1:] // pop off message
			if pos&1 == 0 {         // left side, go to sibling
				pos |= 1
			} // if on right side we don't move; stack ops will move next
			r[n.p] = n
			s = append(s, n) // push new node onto the stack
		}

		// done with pushing onto stack; advance flag bit
		m.bits = m.bits[1:]
	}
	return nil, fmt.Errorf("ran out of things to do?")
}

func (m *merkleNodes) calcTxIndex(txID *common.Uint256) error {
	width := m.calcTreeWidth(0)
	for _, node := range m.allNodes {
		if node.p > width {
			continue
		}
		if *node.h == *txID {
			m.txIndex = node.p
			return nil
		}
	}
	return errors.New("tx index not found")
}

func (m *merkleNodes) calcBranchRoute() {
	depth := treeDepth(m.numTxs)
	for height := uint32(0); height < depth; height++ {
		if m.inDeadZone(height) {
			m.route = append(m.route, m.calcNodeIndex(height, m.txIndex>>height))
			continue
		}
		if m.isLeftLeaf(height) {
			m.route = append(m.route, m.calcNodeIndex(height, (m.txIndex>>height)+1))
		} else {
			m.route = append(m.route, m.calcNodeIndex(height, (m.txIndex>>height)-1))
		}
	}
}

func (m *merkleNodes) inDeadZone(height uint32) bool {
	index := m.txIndex >> height
	return index == m.calcTreeWidth(height)-1 && index%2 == 0
}

func (m *merkleNodes) isLeftLeaf(height uint32) bool {
	index := m.txIndex >> height
	return index%2 == 0
}

func (m *merkleNodes) calcTreeWidth(height uint32) uint32 {
	return (m.numTxs + (1 << height) - 1) >> height
}

func (m *merkleNodes) calcNodeIndex(height, pos uint32) uint32 {
	msb := nextPowerOfTwo(m.numTxs)<<1 - 2
	height = treeDepth(m.numTxs) - height
	for i := uint32(1); i <= height; i++ {
		msb -= 1 << i
	}
	return msb + pos
}
