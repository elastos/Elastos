package blockchain

import (
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	side "github.com/elastos/Elastos.ELA.SideChain/types"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

type IDChainStore struct {
	*blockchain.ChainStore
}

func NewChainStore(path string, genesisBlock *side.Block) (*IDChainStore, error) {
	chainStore, err := blockchain.NewChainStore(path, genesisBlock)
	if err != nil {
		return nil, err
	}

	store := &IDChainStore{
		ChainStore: chainStore,
	}

	store.RegisterFunctions(true, blockchain.StoreFuncNames.PersistTransactions, store.persistTransactions)

	return store, nil
}

func (c *IDChainStore) persistTransactions(batch database.Batch, b *side.Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.Height); err != nil {
			return err
		}

		if txn.TxType == side.RegisterAsset {
			regPayload := txn.Payload.(*side.PayloadRegisterAsset)
			if err := c.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}

		if txn.TxType == side.RechargeToSideChain {
			rechargePayload := txn.Payload.(*side.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			c.PersistMainchainTx(batch, *hash)
		}

		if txn.TxType == side.Deploy {

		}

		if txn.TxType == types.Invoke {

		}
	}
	return nil
}

func (c *IDChainStore) PersistRegisterIdentificationTx(batch database.Batch, idKey []byte, txHash Uint256) {
	key := []byte{byte(blockchain.IX_Identification)}
	key = append(key, idKey...)

	// PUT VALUE
	batch.Put(key, txHash.Bytes())
}

func (c *IDChainStore) GetRegisterIdentificationTx(idKey []byte) ([]byte, error) {
	key := []byte{byte(blockchain.IX_Identification)}
	data, err := c.Get(append(key, idKey...))
	if err != nil {
		return nil, err
	}

	return data, nil
}
