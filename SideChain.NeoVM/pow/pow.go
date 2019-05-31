package pow

import (
	"github.com/elastos/Elastos.ELA.SideChain/pow"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	ntypes "github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"
)

type Config struct {
	*pow.Config

	GenerateBlock             func(cfg *Config) (*types.Block, error)
}

func GenerateBlock(cfg *pow.Config) (*types.Block, error) {
	block, err := pow.GenerateBlock(cfg)
	if err != nil {
		log.Errorf("notifyInfo:", err)
		return block, err
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
	block.ReceiptHash = receipts.Hash()
	block.Bloom = ntypes.CreateBloom(receipts).Bytes()
	return block, err
}