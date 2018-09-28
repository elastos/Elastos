package blockchain

import (
	"github.com/elastos/Elastos.ELA.SideChain/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

// IChainStore provides func with store package.
type IChainStore interface {
	SaveBlock(b *core.Block) error
	GetBlock(hash Uint256) (*core.Block, error)
	GetBlockHash(height uint32) (Uint256, error)
	IsDoubleSpend(tx *core.Transaction) bool

	GetHeader(hash Uint256) (*core.Header, error)

	RollbackBlock(hash Uint256) error

	GetTransaction(txId Uint256) (*core.Transaction, uint32, error)
	GetTxReference(tx *core.Transaction) (map[*core.Input]*core.Output, error)

	GetAsset(hash Uint256) (*core.Asset, error)

	GetMainchainTx(mainchainTxHash Uint256) (byte, error)

	GetCurrentBlockHash() Uint256
	GetHeight() uint32

	GetUnspent(txId Uint256, index uint16) (*core.Output, error)
	ContainsUnspent(txId Uint256, index uint16) (bool, error)
	GetAssetUnspents(programHash Uint168, assetid Uint256) ([]*UTXO, error)
	GetUnspents(programHash Uint168) (map[Uint256][]*UTXO, error)
	GetAssets() map[Uint256]*core.Asset

	IsDuplicateTx(txId Uint256) bool
	IsDuplicateMainchainTx(txId Uint256) bool
	IsBlockInStore(hash Uint256) bool
	Close()
}
