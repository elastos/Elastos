package blockchain

import (
	"bytes"

	"github.com/elastos/Elastos.ELA.SideChain.ID/core"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	ucore "github.com/elastos/Elastos.ELA.SideChain/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type IDChainStore struct {
	blockchain.ChainStore
}

func NewChainStore(genesisBlock *ucore.Block) (blockchain.IChainStore, error) {
	chainStore, err := blockchain.NewChainStore(genesisBlock)
	if err != nil {
		return nil, err
	}

	store := &IDChainStore{
		ChainStore: *chainStore,
	}

	store.RegisterFunctions(true, blockchain.StoreFuncNames.PersistTransactions, store.persistTransactions)

	go store.TaskHandler()

	return store, nil
}

func (c *IDChainStore) persistTransactions(batch blockchain.IBatch, b *ucore.Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.Height); err != nil {
			return err
		}
		if txn.TxType == ucore.RegisterAsset {
			regPayload := txn.Payload.(*ucore.PayloadRegisterAsset)
			if err := c.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		}
		if txn.TxType == ucore.RechargeToSideChain {
			rechargePayload := txn.Payload.(*ucore.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash()
			if err != nil {
				return err
			}
			c.PersistMainchainTx(batch, *hash)
		}
		if txn.TxType == core.RegisterIdentification {
			regPayload := txn.Payload.(*core.PayloadRegisterIdentification)
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

func (c *IDChainStore) PersistRegisterIdentificationTx(batch blockchain.IBatch, idKey []byte, txHash Uint256) {
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
