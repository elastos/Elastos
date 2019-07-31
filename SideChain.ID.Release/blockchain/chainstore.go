package blockchain

import (
	"bytes"

	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
)

const IX_DID blockchain.EntryPrefix = 0x95

type IDChainStore struct {
	*blockchain.ChainStore
}

func NewChainStore(genesisBlock *types.Block, dataPath string) (*IDChainStore, error) {
	chainStore, err := blockchain.NewChainStore(dataPath, genesisBlock)
	if err != nil {
		return nil, err
	}

	store := &IDChainStore{
		ChainStore: chainStore,
	}

	store.RegisterFunctions(true, blockchain.StoreFuncNames.PersistTransactions, store.persistTransactions)
	store.RegisterFunctions(false, blockchain.StoreFuncNames.RollbackTransactions, store.rollbackTransactions)

	return store, nil
}

func (c *IDChainStore) persistTransactions(batch database.Batch, b *types.Block) error {
	for _, txn := range b.Transactions {
		if err := c.PersistTransaction(batch, txn, b.Header.GetHeight()); err != nil {
			return err
		}

		switch txn.TxType {
		case types.RegisterAsset:
			regPayload := txn.Payload.(*types.PayloadRegisterAsset)
			if err := c.PersistAsset(batch, txn.Hash(), regPayload.Asset); err != nil {
				return err
			}
		case types.RechargeToSideChain:
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			c.PersistMainchainTx(batch, *hash)
		case id.RegisterIdentification:
			regPayload := txn.Payload.(*id.PayloadRegisterIdentification)
			for _, content := range regPayload.Contents {
				buf := new(bytes.Buffer)
				buf.WriteString(regPayload.ID)
				buf.WriteString(content.Path)
				c.persistRegisterIdentificationTx(batch, buf.Bytes(), txn.Hash())
			}
		case id.RegisterDID:
			// todo complete me


		case id.UpdateDID:
			// todo complete me
		}
	}
	return nil
}

func (c *IDChainStore) rollbackTransactions(batch database.Batch, b *types.Block) error {
	for _, txn := range b.Transactions {
		if err := c.RollbackTransaction(batch, txn); err != nil {
			return err
		}

		switch txn.TxType {
		case types.RegisterAsset:
			if err := c.RollbackAsset(batch, txn.Hash()); err != nil {
				return err
			}
		case types.RechargeToSideChain:
			rechargePayload := txn.Payload.(*types.PayloadRechargeToSideChain)
			hash, err := rechargePayload.GetMainchainTxHash(txn.PayloadVersion)
			if err != nil {
				return err
			}
			c.RollbackMainchainTx(batch, *hash)
		case id.RegisterIdentification:
			regPayload := txn.Payload.(*id.PayloadRegisterIdentification)
			for _, content := range regPayload.Contents {
				buf := new(bytes.Buffer)
				buf.WriteString(regPayload.ID)
				buf.WriteString(content.Path)
				c.rollbackRegisterIdentificationTx(batch, buf.Bytes())
			}
		case id.RegisterDID:
			// todo complete me
		case id.UpdateDID:
			// tod complete me
		}
	}

	return nil
}

func (c *IDChainStore) persistRegisterIdentificationTx(batch database.Batch, idKey []byte, txHash common.Uint256) {
	key := []byte{byte(blockchain.IX_Identification)}
	key = append(key, idKey...)

	// PUT VALUE
	batch.Put(key, txHash.Bytes())
}

func (c *IDChainStore) rollbackRegisterIdentificationTx(batch database.Batch, idKey []byte) {
	key := []byte{byte(blockchain.IX_Identification)}
	key = append(key, idKey...)

	// PUT VALUE
	batch.Delete(key)
}

func (c *IDChainStore) GetRegisterIdentificationTx(idKey []byte) ([]byte, error) {
	key := []byte{byte(blockchain.IX_Identification)}
	data, err := c.Get(append(key, idKey...))
	if err != nil {
		return nil, err
	}

	return data, nil
}

func (c *IDChainStore) GetLastRegisterDIDTx(idKey []byte) ([]byte, error) {
	// todo get txs list by id, then get first tx
	return nil, nil
}

func (c *IDChainStore) GetRegisterDIDTx(idKey []byte) ([][]byte, error) {
	// todo get txs list by id, then get all txs
	return nil, nil
}
