package blockchain

import (
	"bytes"
	"encoding/hex"
	"errors"

	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
)

const (
	IX_DIDTXHash  blockchain.EntryPrefix = 0x95
	IX_DIDPayload blockchain.EntryPrefix = 0x96
)

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
				if err := c.persistRegisterIdentificationTx(batch,
					buf.Bytes(), txn.Hash()); err != nil {
					return err
				}
			}
		case id.RegisterDID, id.UpdateDID:
			regPayload := txn.Payload.(*id.PayloadDIDInfo)
			id, _ := hex.DecodeString(regPayload.PayloadInfo.ID)
			if err := c.persistRegisterDIDTx(batch, id, txn); err != nil {
				return err
			}
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
				err := c.rollbackRegisterIdentificationTx(batch, buf.Bytes())
				if err != nil {
					return err
				}
			}
		case id.RegisterDID, id.UpdateDID:
			regPayload := txn.Payload.(*id.PayloadDIDInfo)
			id, _ := hex.DecodeString(regPayload.PayloadInfo.ID)
			if err := c.rollbackRegisterDIDTx(batch, id, txn); err != nil {
				return err
			}
		}
	}

	return nil
}

func (c *IDChainStore) persistRegisterIdentificationTx(batch database.Batch,
	idKey []byte, txHash common.Uint256) error {
	key := []byte{byte(blockchain.IX_Identification)}
	key = append(key, idKey...)

	// PUT VALUE
	return batch.Put(key, txHash.Bytes())
}

func (c *IDChainStore) rollbackRegisterIdentificationTx(batch database.Batch,
	idKey []byte) error {
	key := []byte{byte(blockchain.IX_Identification)}
	key = append(key, idKey...)

	// PUT VALUE
	return batch.Delete(key)
}

func (c *IDChainStore) GetRegisterIdentificationTx(idKey []byte) ([]byte, error) {
	key := []byte{byte(blockchain.IX_Identification)}
	data, err := c.Get(append(key, idKey...))
	if err != nil {
		return nil, err
	}

	return data, nil
}

func (c *IDChainStore) persistRegisterDIDTx(batch database.Batch,
	idKey []byte, tx *types.Transaction) error {
	if err := c.persistRegisterDIDTxHash(batch, idKey, tx.Hash()); err != nil {
		return err
	}

	if err := c.persistRegisterDIDPayload(batch, tx.Hash(),
		tx.Payload.(*id.PayloadDIDInfo)); err != nil {
		return err
	}

	return nil
}

func (c *IDChainStore) persistRegisterDIDTxHash(batch database.Batch,
	idKey []byte, txHash common.Uint256) error {
	key := []byte{byte(IX_DIDTXHash)}
	key = append(key, idKey...)

	data, err := c.Get(key)
	if err != nil {
		// when not exist, only put the current payload hash into db.
		buf := new(bytes.Buffer)
		if err := common.WriteVarUint(buf, 1); err != nil {
			return err
		}

		if err := txHash.Serialize(buf); err != nil {
			return err
		}

		return batch.Put(key, buf.Bytes())
	}

	// when exist, should add current payload hash to the end of the list.
	r := bytes.NewReader(data)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	count++

	buf := new(bytes.Buffer)

	// write count
	if err := common.WriteVarUint(buf, count); err != nil {
		return err
	}

	// write current payload hash
	if err := txHash.Serialize(buf); err != nil {
		return err
	}

	// write old hashes
	if _, err := r.WriteTo(buf); err != nil {
		return err
	}

	return batch.Put(key, buf.Bytes())
}

func (c *IDChainStore) rollbackRegisterDIDTx(batch database.Batch,
	idKey []byte, tx *types.Transaction) error {
	key := []byte{byte(IX_DIDTXHash)}
	key = append(key, idKey...)

	data, err := c.Get(key)
	if err != nil {
		return err
	}

	r := bytes.NewReader(data)
	// get the count of tx hashes
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}
	if count == 0 {
		return errors.New("not exist")
	}
	// get the newest tx hash
	var txHash common.Uint256
	if err := txHash.Deserialize(r); err != nil {
		return err
	}
	// if not rollback the newest tx hash return error
	if !txHash.IsEqual(tx.Hash()) {
		return errors.New("not rollback the last one")
	}

	keyPayload := []byte{byte(IX_DIDPayload)}
	keyPayload = append(keyPayload, txHash.Bytes()...)
	batch.Delete(keyPayload)

	if count == 1 {
		return batch.Delete(key)
	}

	buf := new(bytes.Buffer)

	// write count
	if err := common.WriteVarUint(buf, count-1); err != nil {
		return err
	}

	// write old hashes
	if _, err := r.WriteTo(buf); err != nil {
		return err
	}

	return batch.Put(key, buf.Bytes())
}

func (c *IDChainStore) persistRegisterDIDPayload(batch database.Batch,
	txHash common.Uint256, p *id.PayloadDIDInfo) error {
	key := []byte{byte(IX_DIDPayload)}
	key = append(key, txHash.Bytes()...)

	buf := new(bytes.Buffer)
	p.Serialize(buf, id.DIDInfoVersion)
	return batch.Put(key, buf.Bytes())
}

func (c *IDChainStore) GetLastDIDTxPayload(idKey []byte) ([]byte, error) {
	key := []byte{byte(IX_DIDTXHash)}
	key = append(key, idKey...)

	data, err := c.Get(key)
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(data)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	if count == 0 {
		return nil, errors.New("not exist")
	}
	var txHash common.Uint256
	if err := txHash.Deserialize(r); err != nil {
		return nil, err
	}

	keyPayload := []byte{byte(IX_DIDPayload)}
	keyPayload = append(keyPayload, txHash.Bytes()...)

	dataPayload, err := c.Get(keyPayload)
	if err != nil {
		return nil, err
	}

	return dataPayload, nil
}

func (c *IDChainStore) GetDIDTxPayload(idKey []byte) ([][]byte, error) {
	key := []byte{byte(IX_DIDTXHash)}
	key = append(key, idKey...)

	data, err := c.Get(key)
	if err != nil {
		return nil, err
	}

	r := bytes.NewReader(data)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	var payloadList [][]byte
	for i := uint64(0); i < count; i++ {
		var txHash common.Uint256
		if err := txHash.Deserialize(r); err != nil {
			return nil, err
		}

		keyPayload := []byte{byte(IX_DIDPayload)}
		keyPayload = append(keyPayload, txHash.Bytes()...)

		payloadData, err := c.Get(keyPayload)
		if err != nil {
			return nil, err
		}
		payloadList = append(payloadList, payloadData)
	}

	return payloadList, nil
}
