package blockchain

import (
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type ChainStoreMock struct {
	RegisterProducers []*payload.PayloadRegisterProducer
	BlockHeight       uint32
}

func (c *ChainStoreMock) GetRegisteredProducers() []*payload.PayloadRegisterProducer {
	return c.RegisterProducers
}

func (c *ChainStoreMock) GetRegisteredProducersSorted() ([]*payload.PayloadRegisterProducer, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetProducerVote(publicKey []byte) common.Fixed64 {
	panic("implement me")
}

func (c *ChainStoreMock) GetProducerStatus(address string) ProducerState {
	panic("implement me")
}

func (c *ChainStoreMock) GetIllegalProducers() map[string]struct{} {
	panic("implement me")
}

func (c *ChainStoreMock) GetCancelProducerHeight(publicKey []byte) (uint32, error) {
	panic("implement me")
}

func (c *ChainStoreMock) OnIllegalBlockTxnReceived(txn *types.Transaction) {
	panic("implement me")
}

func (c *ChainStoreMock) InitWithGenesisBlock(genesisblock *types.Block) (uint32, error) {
	panic("implement me")
}

func (c *ChainStoreMock) InitProducerVotes() error {
	panic("implement me")
}

func (c *ChainStoreMock) SaveBlock(b *types.Block) error {
	panic("implement me")
}

func (c *ChainStoreMock) GetBlock(hash common.Uint256) (*types.Block, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetBlockHash(height uint32) (common.Uint256, error) {
	panic("implement me")
}

func (c *ChainStoreMock) IsDoubleSpend(tx *types.Transaction) bool {
	panic("implement me")
}

func (c *ChainStoreMock) SaveConfirm(confirm *types.DPosProposalVoteSlot) error {
	panic("implement me")
}

func (c *ChainStoreMock) GetConfirm(hash common.Uint256) (*types.DPosProposalVoteSlot, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetHeader(hash common.Uint256) (*types.Header, error) {
	panic("implement me")
}

func (c *ChainStoreMock) RollbackBlock(hash common.Uint256) error {
	panic("implement me")
}

func (c *ChainStoreMock) GetTransaction(txID common.Uint256) (*types.Transaction, uint32, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetTxReference(tx *types.Transaction) (map[*types.Input]*types.Output, error) {
	panic("implement me")
}

func (c *ChainStoreMock) PersistAsset(assetid common.Uint256, asset payload.Asset) error {
	panic("implement me")
}

func (c *ChainStoreMock) GetAsset(hash common.Uint256) (*payload.Asset, error) {
	panic("implement me")
}

func (c *ChainStoreMock) PersistSidechainTx(sidechainTxHash common.Uint256) {
	panic("implement me")
}

func (c *ChainStoreMock) GetSidechainTx(sidechainTxHash common.Uint256) (byte, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetCurrentBlockHash() common.Uint256 {
	panic("implement me")
}

func (c *ChainStoreMock) GetHeight() uint32 {
	return c.BlockHeight
}

func (c *ChainStoreMock) GetUnspent(txID common.Uint256, index uint16) (*types.Output, error) {
	panic("implement me")
}

func (c *ChainStoreMock) ContainsUnspent(txID common.Uint256, index uint16) (bool, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetUnspentFromProgramHash(programHash common.Uint168, assetid common.Uint256) ([]*UTXO, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetUnspentsFromProgramHash(programHash common.Uint168) (map[common.Uint256][]*UTXO, error) {
	panic("implement me")
}

func (c *ChainStoreMock) GetAssets() map[common.Uint256]*payload.Asset {
	panic("implement me")
}

func (c *ChainStoreMock) IsTxHashDuplicate(txhash common.Uint256) bool {
	panic("implement me")
}

func (c *ChainStoreMock) IsSidechainTxHashDuplicate(sidechainTxHash common.Uint256) bool {
	panic("implement me")
}

func (c *ChainStoreMock) IsBlockInStore(hash common.Uint256) bool {
	panic("implement me")
}

func (c *ChainStoreMock) Close() {
	panic("implement me")
}
