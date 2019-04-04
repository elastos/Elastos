package blockchain

import (
	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

// IChainStore provides func with store package.
type IChainStore interface {
	SaveBlock(b *Block, confirm *payload.Confirm) error
	GetBlock(hash Uint256) (*Block, error)
	GetBlockHash(height uint32) (Uint256, error)
	IsDoubleSpend(tx *Transaction) bool

	GetConfirm(hash Uint256) (*payload.Confirm, error)

	GetHeader(hash Uint256) (*Header, error)

	RollbackBlock(hash Uint256) error

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
