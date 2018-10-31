package spv

import (
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

type txStore struct {
	db *blockchain.ChainStore
}

func (s *txStore) Put(tx *core.Transaction) error {
	batch := s.db.NewBatch()
	s.db.PersistSpvMainchainTx(batch, tx)
	return batch.Commit()
}

func (s *txStore) Get(txId *common.Uint256) (*core.Transaction, error) {
	return s.db.GetSpvMainchainTx(txId)
}

func NewTxStore(db *blockchain.ChainStore) *txStore {
	return &txStore{db: db}
}
