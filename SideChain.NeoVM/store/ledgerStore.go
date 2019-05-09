package store

import (
	"errors"

	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	side "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

var (
	ErrDBNotFound = errors.New("leveldb: not found")
)

type LedgerStore struct {
	*sb.ChainStore
}

func NewLedgerStore(store *sb.ChainStore) (*LedgerStore, error) {
	ledger := &LedgerStore{
		ChainStore: store,
	}
	ledger.RegisterFunctions(true, sb.StoreFuncNames.PersistTransactions, ledger.persistTransactions)
	ledger.RegisterFunctions(true, PersisAccount, ledger.PersisAccount)

	return ledger, nil
}

func (c *LedgerStore) persistTransactions(batch database.Batch, b *side.Block) error {
	c.WriteTxLookupEntries(b)
	var receipts types.Receipts
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
			err := c.PersistDeployTransaction(b, txn, batch)
			if err != nil {
				log.Error(err.Error())
				//return err will effect manual mining
				return nil
			}
		}

		if txn.TxType == side.Invoke {
			receipt, err := c.PersisInvokeTransaction(b, txn, batch)
			if err != nil {
				log.Error(err.Error())
				//return err will effect manual mining
				return nil
			}
			receipts = append(receipts, receipt)
		}
	}
	if len(receipts) > 0 {
		err := c.WriteReceipts(b, receipts)
		if err != nil {
			log.Error("WriteReceipts errors:", err)
		}
	}
	return nil
}

func (c *LedgerStore) GetUnspents(txid common.Uint256) ([]*side.Output, error) {
	if ok, _ := c.ContainsUnspent(txid, 0); ok {
		tx, _, err := c.GetTransaction(txid)
		if err != nil {
			return nil, err
		}
		return tx.Outputs, nil
	}
	return nil, errors.New("[GetUnspent] NOT ContainsUnspent.")
}
