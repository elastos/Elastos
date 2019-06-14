package blockchain

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/interfaces"

	"github.com/elastos/Elastos.ELA/common"
)

var DefaultChain *AVMChain

type AVMChain struct {
	*blockchain.BlockChain
	Validator *mempool.Validator
	Store   ILedgerStore
}

func NewBlockChain(cfg *blockchain.Config, valditator *mempool.Validator, store ILedgerStore) (*AVMChain, error) {
	bc, err := blockchain.New(cfg)
	if err != nil {
		return nil, err
	}

	var avmChain = new(AVMChain)
	avmChain.BlockChain = bc
	avmChain.Validator =  valditator
	avmChain.Store = store
	return avmChain, err
}

//Get block with block hash.
func (b *AVMChain) GetBlockByHash(hash common.Uint256) (*types.Block, error) {
	bk, err := b.Store.GetBlock(hash)
	if err != nil {
		return nil, errors.New("[Ledger],GetBlockWithHeight failed with hash=" + hash.String())
	}
	return bk, nil
}

func (b *AVMChain) GetHeader(hash common.Uint256) (interfaces.Header, error) {
	header, err := b.Store.GetHeader(hash)
	if err != nil {
		return nil, errors.New("[BlockChain], GetHeader failed.")
	}
	return header, nil
}