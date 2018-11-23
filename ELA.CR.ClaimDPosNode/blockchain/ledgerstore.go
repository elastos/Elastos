package blockchain

import (
	. "github.com/elastos/Elastos.ELA/core"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core/outputpayload"
)

// IChainStore provides func with store package.
type IChainStore interface {
	InitWithGenesisBlock(genesisblock *Block) (uint32, error)
	InitProducerVotes() error

	SaveBlock(b *Block) error
	GetBlock(hash Uint256) (*Block, error)
	GetBlockHash(height uint32) (Uint256, error)
	IsDoubleSpend(tx *Transaction) bool

	SaveConfirm(confirm *DPosProposalVoteSlot) error

	GetHeader(hash Uint256) (*Header, error)

	RollbackBlock(hash Uint256) error

	GetTransaction(txId Uint256) (*Transaction, uint32, error)
	GetTxReference(tx *Transaction) (map[*Input]*Output, error)

	PersistAsset(assetid Uint256, asset Asset) error
	GetAsset(hash Uint256) (*Asset, error)

	PersistSidechainTx(sidechainTxHash Uint256)
	GetSidechainTx(sidechainTxHash Uint256) (byte, error)

	GetRegisteredProducers() []*PayloadRegisterProducer
	GetProducerVote(voteType outputpayload.VoteType, programHash Uint168) Fixed64
	GetProducerStatus(programHash Uint168) ProducerState

	GetCurrentBlockHash() Uint256
	GetHeight() uint32

	RemoveHeaderListElement(hash Uint256)

	GetUnspent(txid Uint256, index uint16) (*Output, error)
	ContainsUnspent(txid Uint256, index uint16) (bool, error)
	GetUnspentFromProgramHash(programHash Uint168, assetid Uint256) ([]*UTXO, error)
	GetUnspentsFromProgramHash(programHash Uint168) (map[Uint256][]*UTXO, error)
	GetAssets() map[Uint256]*Asset

	IsTxHashDuplicate(txhash Uint256) bool
	IsSidechainTxHashDuplicate(sidechainTxHash Uint256) bool
	IsBlockInStore(hash Uint256) bool
	Close()
}
