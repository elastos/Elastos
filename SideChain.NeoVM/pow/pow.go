package pow

import (
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	ntypes "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"

	"github.com/elastos/Elastos.ELA/common"
)

type Config struct {
	*pow.Config

	GenerateBlock             func(cfg *Config) (*types.Block, error)
}

func GenerateBlock(cfg *pow.Config) (*types.Block, error) {
	log.Info("[GenerateBlock] int Neo POW :")
	block, err := pow.GenerateBlock(cfg)
	if err != nil {
		log.Errorf("[GenerateBlock] in Neo POW Error :%s", err.Error())
		return block, err
	}

	h := types.Header{
		Version:    0,
		Previous:   block.GetPrevious(),
		MerkleRoot: block.GetMerkleRoot(),
		Timestamp:  block.GetTimeStamp(),
		Bits:       block.GetBits(),
		Height:     block.GetHeight(),
		Nonce:      block.GetNonce(),
	}

	header := &ntypes.Header{
	 	Header: &h,
	 	ReceiptHash: common.EmptyHash,
	}
	storedb := blockchain.DefaultChain.Store.(*store.LedgerStore)

	var receipts ntypes.Receipts
	for _, txn := range block.Transactions {
		if txn.TxType == types.Invoke {
			receipt, err := storedb.PersisInvokeTransaction(block, txn, nil)
			if err != nil {
				log.Error("GenerateBlock Invoke failed")
			}
			receipts = append(receipts, receipt)
		}
	}
	header.ReceiptHash = receipts.Hash()
	header.Bloom = ntypes.CreateBloom(receipts).Bytes()

	block.Header = header
	log.Info("[GenerateBlock] int Neo POW Success :")
	return block, err
}