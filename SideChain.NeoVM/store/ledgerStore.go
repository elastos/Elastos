package store

import (
	"errors"

	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"
	side "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
	"github.com/elastos/Elastos.ELA.Utility/common"
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
				return err
			}

		}

		if txn.TxType == types.Invoke {
			err := c.persisInvokeTransaction(b, txn, batch)
			if err != nil {
				log.Error(err.Error())
				return err
			}
		}
	}
	return nil
}

func (c *LedgerStore) GetUnspents(txid common.Uint256) ([]*side.Output, error) {
	return c.GetUnspents(txid)
}
