// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

// Copyright (c) 2013-2016 The btcsuite developers
// Copyright (c) 2017-2019 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/database"
)

// blockStatus is a bit field representing the validation state of the block.
type blockStatus byte

const (
	// statusDataStored indicates that the block's payload is stored on disk.
	statusDataStored blockStatus = 1 << iota

	// statusValid indicates that the block has been fully validated.
	statusValid

	// statusValidateFailed indicates that the block has failed validation.
	statusValidateFailed

	// statusInvalidAncestor indicates that one of the block's ancestors has
	// has failed validation, thus the block is also invalid.
	statusInvalidAncestor

	// statusNone indicates that the block has no validation state flags set.
	//
	// NOTE: This must be defined last in order to avoid influencing iota.
	statusNone blockStatus = 0
)

// HaveData returns whether the full block data is stored in the database. This
// will return false for a block node where only the header is downloaded or
// kept.
func (status blockStatus) HaveData() bool {
	return status&statusDataStored != 0
}

// KnownValid returns whether the block is known to be valid. This will return
// false for a valid block that has not been fully validated yet.
func (status blockStatus) KnownValid() bool {
	return status&statusValid != 0
}

// KnownInvalid returns whether the block is known to be invalid. This may be
// because the block itself failed validation or any of its ancestors is
// invalid. This will return false for invalid blocks that have not been proven
// invalid yet.
func (status blockStatus) KnownInvalid() bool {
	return status&(statusValidateFailed|statusInvalidAncestor) != 0
}

// BlockNode represents a block within the block chain and is primarily used to
// aid in selecting the best chain to be the main chain.  The main chain is
// stored into the block database.
type BlockNode struct {
	Hash        *common.Uint256
	ParentHash  *common.Uint256
	Height      uint32
	Version     uint32
	Bits        uint32
	Timestamp   uint32
	WorkSum     *big.Int
	InMainChain bool
	Parent      *BlockNode
	Children    []*BlockNode

	// Status is a bitfield representing the validation state of the block. The
	// Status field, unlike the other fields, may be written to and so should
	// only be accessed using the concurrent-safe NodeStatus method on
	// blockIndex once the node has been added to the global index.
	Status blockStatus
}

// NewBlockNode returns a new block node for the given block header and block
// hash, calculating the height and workSum from the respective fields.
func NewBlockNode(header *types.Header, hash *common.Uint256) *BlockNode {
	var previous, current common.Uint256
	copy(previous[:], header.Previous[:])
	copy(current[:], hash[:])
	node := BlockNode{
		Hash:       &current,
		ParentHash: &previous,
		Height:     header.Height,
		Version:    header.Version,
		Bits:       header.Bits,
		Timestamp:  header.Timestamp,
		WorkSum:    CalcWork(header.Bits),
	}
	return &node
}

// blockIndex provides facilities for keeping track of an in-memory index of the
// block chain.  Although the name block chain suggests a single chain of
// blocks, it is actually a tree-shaped structure where any node can have
// multiple children.  However, there can only be one active branch which does
// indeed form a chain from the tip all the way back to the genesis block.
type blockIndex struct {
	// The following fields are set when the instance is created and can't
	// be changed afterwards, so there is no need to protect them with a
	// separate mutex.
	db          IChainStore
	chainParams *config.Params

	sync.RWMutex
	index map[common.Uint256]*BlockNode
	dirty map[*types.Header]blockStatus
}

// newBlockIndex returns a new empty instance of a block index.  The index will
// be dynamically populated as block nodes are loaded from the database and
// manually added.
func newBlockIndex(db IChainStore, chainParams *config.Params) *blockIndex {
	return &blockIndex{
		db:          db,
		chainParams: chainParams,
		index:       make(map[common.Uint256]*BlockNode),
		dirty:       make(map[*types.Header]blockStatus),
	}
}

// HaveBlock returns whether or not the block index contains the provided hash.
//
// This function is safe for concurrent access.
func (bi *blockIndex) HaveBlock(hash *common.Uint256) bool {
	bi.RLock()
	_, hasBlock := bi.index[*hash]
	bi.RUnlock()
	return hasBlock
}

// LookupNode returns the block node identified by the provided hash.  It will
// return nil if there is no entry for the hash.
//
// This function is safe for concurrent access.
func (bi *blockIndex) LookupNode(hash *common.Uint256) (*BlockNode, bool) {
	bi.RLock()
	node, exist := bi.index[*hash]
	bi.RUnlock()
	return node, exist
}

// AddNode adds the provided node to the block index and marks it as dirty.
// Duplicate entries are not checked so it is up to caller to avoid adding them.
//
// This function is safe for concurrent access.
func (bi *blockIndex) AddNode(node *BlockNode, header *types.Header) {
	bi.Lock()
	bi.addNode(node)
	bi.dirty[header] = node.Status
	bi.Unlock()
}

// addNode adds the provided node to the block index, but does not mark it as
// dirty. This can be used while initializing the block index.
//
// This function is NOT safe for concurrent access.
func (bi *blockIndex) addNode(node *BlockNode) {
	bi.index[*node.Hash] = node
}

func (bi *blockIndex) RemoveNode(node *BlockNode) {
	bi.Lock()
	delete(bi.index, *node.Hash)
	bi.Unlock()
}

// NodeStatus provides concurrent-safe access to the Status field of a node.
//
// This function is safe for concurrent access.
func (bi *blockIndex) NodeStatus(node *BlockNode) blockStatus {
	bi.RLock()
	status := node.Status
	bi.RUnlock()
	return status
}

// SetFlags set the Status flags.
func (bi *blockIndex) SetFlags(header *types.Header, flags blockStatus) {
	bi.Lock()
	bi.dirty[header] = flags
	bi.Unlock()
}

// flushToDB writes all dirty block nodes to the database. If all writes
// succeed, this clears the dirty set.
func (bi *blockIndex) flushToDB() error {
	bi.Lock()
	if len(bi.dirty) == 0 {
		bi.Unlock()
		return nil
	}

	err := bi.db.GetFFLDB().Update(func(dbTx database.Tx) error {
		for header, v := range bi.dirty {
			err := DBStoreBlockNode(dbTx, header, v)
			if err != nil {
				return err
			}
		}
		return nil
	})

	// If write was successful, clear the dirty set.
	if err == nil {
		bi.dirty = make(map[*types.Header]blockStatus)
	}

	bi.Unlock()
	return err
}
