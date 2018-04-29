package blockchain

import (
	"github.com/elastos/Elastos.ELA.SideChain/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	ela "github.com/elastos/Elastos.ELA/core"
)

// IChainStore provides func with store package.
type IChainStore interface {
	InitWithGenesisBlock(genesisblock *core.Block) (uint32, error)

	SaveBlock(b *core.Block) error
	GetBlock(hash Uint256) (*core.Block, error)
	BlockInCache(hash Uint256) bool
	GetBlockHash(height uint32) (Uint256, error)
	IsDoubleSpend(tx *ela.Transaction) bool

	GetHeader(hash Uint256) (*core.Header, error)

	RollbackBlock(hash Uint256) error

	GetTransaction(txId Uint256) (*ela.Transaction, uint32, error)
	GetTxReference(tx *ela.Transaction) (map[*ela.Input]*ela.Output, error)

	PersistAsset(assetid Uint256, asset ela.Asset) error
	GetAsset(hash Uint256) (*ela.Asset, error)

	GetCurrentBlockHash() Uint256
	GetHeight() uint32

	RemoveHeaderListElement(hash Uint256)

	GetUnspent(txid Uint256, index uint16) (*ela.Output, error)
	ContainsUnspent(txid Uint256, index uint16) (bool, error)
	GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*UTXO, error)
	GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*UTXO, error)
	GetAssets() map[Uint256]*ela.Asset

	IsTxHashDuplicate(txhash Uint256) bool
	IsBlockInStore(hash Uint256) bool
	Close()
}
