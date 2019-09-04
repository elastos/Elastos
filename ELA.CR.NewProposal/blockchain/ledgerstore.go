// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package blockchain

import (
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/database"
)

// IChainStore provides func with store package.
type IChainStore interface {
	GetFFLDB() IFFLDBChainStore

	SaveBlock(b *Block, node *BlockNode, confirm *payload.Confirm,
		medianTimePast time.Time) error
	GetBlock(hash Uint256) (*Block, error)
	GetBlockHash(height uint32) (Uint256, error)
	IsDoubleSpend(tx *Transaction) bool

	GetConfirm(hash Uint256) (*payload.Confirm, error)

	GetHeader(hash Uint256) (*Header, error)

	RollbackBlock(b *Block, node *BlockNode,
		confirm *payload.Confirm, medianTimePast time.Time) error

	GetTransaction(txID Uint256) (*Transaction, uint32, error)
	GetTxReference(tx *Transaction) (map[*Input]*Output, error)

	PersistAsset(assetid Uint256, asset payload.Asset) error
	GetAsset(hash Uint256) (*payload.Asset, error)

	PersistSidechainTx(sidechainTxHash Uint256)
	GetSidechainTx(sidechainTxHash Uint256) (byte, error)

	GetCurrentBlockHash() Uint256
	GetHeight() uint32

	GetUnspent(txID Uint256, index uint16) (*Output, error)
	ContainsUnspent(txID Uint256, index uint16) (bool, error)
	GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*UTXO, error)
	GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*UTXO, error)
	GetAssets() map[Uint256]*payload.Asset

	IsTxHashDuplicate(txhash Uint256) bool
	IsSidechainTxHashDuplicate(sidechainTxHash Uint256) bool
	IsBlockInStore(hash *Uint256) bool

	Close()
}

// IChainStore provides func with store package.
type IFFLDBChainStore interface {
	database.DB

	// SaveBlock will write block into file db.
	SaveBlock(b *Block, node *BlockNode, confirm *payload.Confirm,
		medianTimePast time.Time) error

	// RollbackBlock only remove block state and block index.
	RollbackBlock(b *Block, node *BlockNode,
		confirm *payload.Confirm, medianTimePast time.Time) error

	// Get block from file db.
	GetBlock(hash Uint256) (*Block, error)

	// Get block header from file db.
	GetHeader(hash Uint256) (*Header, error)

	// If already exist in main chain(exist in file db and exist block index),
	// will return true.
	BlockExists(hash *Uint256) (bool, error)

	// If already exist in file db (rollback will not remove from file db), will
	// return true.
	IsBlockInStore(hash *Uint256) bool
}
