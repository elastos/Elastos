package blockchain

import (
	"bytes"

	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type IDChainStore struct {
	*blockchain.ChainStore
}

func NewChainStore(genesisBlock *types.Block) (*IDChainStore, error) {
	chainStore, err := blockchain.NewChainStore(genesisBlock)
	if err != nil {
		return nil, err
	}

	store := &IDChainStore{
		ChainStore: chainStore,
	}

	store.RegisterFunctions(true, blockchain.StoreFuncNames.PersistTransactions, store.persistTransactions)

	return store, nil
}

func (c *IDChainStore) persistTransactions(batch database.Batch, b *types.Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.Height); err != nil {
			return err
		}

		if txn.TxType == types.RegisterAsset {
			regPayload := txn.Payload.(*types.PayloadRegisterAsset)
			if err := c.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}

		if txn.TxType == types.RechargeToSideChain {
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash()
			if err != nil {
				return err
			}
			c.PersistMainchainTx(batch, *hash)
		}

		if txn.TxType == id.RegisterIdentification {
			regPayload := txn.Payload.(*id.PayloadRegisterIdentification)
			for _, content := range regPayload.Contents {
				buf := new(bytes.Buffer)
				buf.WriteString(regPayload.ID)
				buf.WriteString(content.Path)
				c.PersistRegisterIdentificationTx(batch, buf.Bytes(), txn.Hash())
			}
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
