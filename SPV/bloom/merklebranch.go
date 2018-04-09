package bloom

import (
	"errors"
	"fmt"
	"math"
	"sort"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

type MerkleBranch struct {
	Branches []Uint256
	Index    int
}

func (msg *MerkleBlock) GetTxMerkleBranch(txId *Uint256) (*MerkleBranch, error) {
	nodes, err := msg.getMerkleNodes()
	if err != nil {
		return nil, err
	}
	fmt.Println("All nodes:", nodes)
	txIndex := msg.findTxIndex(nodes, txId)
	if txIndex == math.MaxUint32 {
		return nil, errors.New("Can not find transaction with hash: " + txId.String())
	}

	msg.prune(nodes, uint32(treeDepth(msg.Transactions)), 0, txIndex)
	fmt.Println("Pruned nodes:", nodes)

	indexes := make([]int, 0, len(nodes))
	for _, node := range nodes {
		indexes = append(indexes, int(node.p))
	}
	sort.Ints(indexes)

	return getMerkleBranch(indexes, nodes), nil
}

func getMerkleBranch(indexes []int, nodes map[uint32]merkleNode) *MerkleBranch {
	mb := new(MerkleBranch)
	for i, index := range indexes {
		node := nodes[uint32(index)]
		mb.Branches = append(mb.Branches, *node.h)
		if node.p%2 == 0 {
			mb.Index += 1 << uint32(i)
			continue
		}
	}
	return mb
}

func (msg *MerkleBlock) getMerkleNodes() (map[uint32]merkleNode, error) {
	if msg.Transactions == 0 {
		return nil, fmt.Errorf("No transactions in merkleblock")
	}
	if len(msg.Flags) == 0 {
		return nil, fmt.Errorf("No flag bits")
	}
	var s []merkleNode                  // the stack
	var r = make(map[uint32]merkleNode) // slice to return; txids we care about

	// set initial position to root of merkle tree
	msb := nextPowerOfTwo(msg.Transactions) // most significant bit possible
	pos := (msb << 1) - 2                   // current position in tree

	var i uint8 // position in the current flag byte
	var tip int
	// main loop
	for {
		tip = len(s) - 1 // slice position of stack tip
		// First check if stack operations can be performed
		// is stack one filled item?  that's complete.
		if tip == 0 && s[0].h != nil {
			if s[0].h.IsEqual(&msg.BlockHeader.MerkleRoot) {
				return r, nil
			}
			return nil, fmt.Errorf("computed root %s but expect %s\n",
				s[0].h.String(), msg.BlockHeader.MerkleRoot.String())
		}
		// is current position in the tree's dead zone? partial parent
		if inDeadZone(pos, msg.Transactions) {
			// create merkle parent from single side (left)
			h, err := MakeMerkleParent(s[tip].h, nil)
			if err != nil {
				return r, err
			}
			s[tip-1].h = h
			r[s[tip-1].p] = s[tip-1]
			s = s[:tip]          // remove 1 from stack
			pos = s[tip-1].p | 1 // move position to parent's sibling
			continue
		}
		// does stack have 3+ items? and are last 2 items filled?
		if tip > 1 && s[tip-1].h != nil && s[tip].h != nil {
			//fmt.Printf("nodes %d and %d combine into %d\n",
			//	s[tip-1].p, s[tip].p, s[tip-2].p)
			// combine two filled nodes into parent node
			h, err := MakeMerkleParent(s[tip-1].h, s[tip].h)
			if err != nil {
				return r, err
			}
			s[tip-2].h = h
			r[s[tip-2].p] = s[tip-2]
			// remove children
			s = s[:tip-1]
			// move position to parent's sibling
			pos = s[tip-2].p | 1
			continue
		}

		// no stack ops to perform, so make new node from message hashes
		if len(msg.Hashes) == 0 {
			return nil, fmt.Errorf("Ran out of hashes at position %d.", pos)
		}
		if len(msg.Flags) == 0 {
			return nil, fmt.Errorf("Ran out of flag bits.")
		}
		var n merkleNode // make new node
		n.p = pos        // set current position for new node

		if pos&msb != 0 { // upper non-txid hash
			if msg.Flags[0]&(1<<i) == 0 { // flag bit says fill node
				n.h = msg.Hashes[0]         // copy hash from message
				msg.Hashes = msg.Hashes[1:] // pop off message
				if pos&1 != 0 { // right side; ascend
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
			if pos >= msg.Transactions {
				// this can't happen because we check deadzone above...
				return nil, fmt.Errorf("got into an invalid txid node")
			}
			n.h = msg.Hashes[0]         // copy hash from message
			msg.Hashes = msg.Hashes[1:] // pop off message
			if pos&1 == 0 { // left side, go to sibling
				pos |= 1
			}                           // if on right side we don't move; stack ops will move next
			r[n.p] = n
			s = append(s, n) // push new node onto the stack
		}

		// done with pushing onto stack; advance flag bit
		i++
		if i == 8 { // move to next byte
			i = 0
			msg.Flags = msg.Flags[1:]
		}
	}
	return nil, fmt.Errorf("ran out of things to do?")
}

func (msg *MerkleBlock) findTxIndex(nodes map[uint32]merkleNode, txId *Uint256) uint32 {
	for _, node := range nodes {
		if node.p > msg.calcTreeWidth(0) {
			continue
		}
		if node.h.IsEqual(txId) {
			return node.p
		}
	}
	return math.MaxUint32
}

func (msg *MerkleBlock) calcTreeWidth(height uint32) uint32 {
	return (msg.Transactions + (1 << height) - 1) >> height
}

func (msg *MerkleBlock) prune(nodes map[uint32]merkleNode, height, pos, index uint32) {
	var isParent byte
	for i := pos << height; i < (pos+1)<<height && i < msg.Transactions; i++ {
		if i == uint32(index) {
			isParent |= 0x01
			break
		}
	}

	nIndex := msg.calcNodeIndex(height, pos)
	if (height == 0 || isParent == 0x00) && nIndex != uint32(index) {
		return
	}
	delete(nodes, nIndex)

	msg.prune(nodes, height-1, pos*2, index)

	if pos*2+1 < msg.calcTreeWidth(height-1) {
		msg.prune(nodes, height-1, pos*2+1, index)
	}
}

func (msg *MerkleBlock) calcNodeIndex(height, pos uint32) uint32 {
	msb := nextPowerOfTwo(msg.Transactions)<<1 - 2
	height = uint32(treeDepth(msg.Transactions)) - height
	for i := uint32(1); i <= height; i++ {
		msb -= 1 << i
	}
	return msb + pos
}
