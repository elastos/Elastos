package blockchain

import (
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
)

var DefaultChain *AVMChain

type AVMChain struct {
	*blockchain.BlockChain
	Validator *mempool.Validator
	Store   ILedgerStore
}

func NewBlockChain(cfg *blockchain.Config, valditator *mempool.Validator) (*AVMChain, error) {
	bc, err := blockchain.New(cfg)
	if err != nil {
		return nil, err
	}

	var avmChain = new(AVMChain)
	avmChain.BlockChain = bc
	avmChain.Validator =  valditator
	return avmChain, err
}