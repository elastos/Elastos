package store

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	sb "github.com/elastos/Elastos.ELA.SideChain/blockchain"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/blockchain"
)

type CacheCodeTable struct {
	dbCache *blockchain.DBCache
}

func NewCacheCodeTable(dbCache *blockchain.DBCache) *CacheCodeTable {
	return &CacheCodeTable{dbCache: dbCache}
}

func (table *CacheCodeTable) GetScript(codeHash []byte) ([]byte) {
	value, err := table.dbCache.TryGet(sb.ST_Contract, string(codeHash))
	if err != nil {
		return nil
	}
	return value.(*states.ContractState).Code.Code
}

func (table *CacheCodeTable) GetTxReference(tx *interfaces.IDataContainer) (map[*types.Input]*types.Output, error) {
	txn := (*tx).(*types.Transaction)
	store := table.dbCache.GetChainStoreDb()
	chainStore := store.(*LedgerStore)
	if chainStore == nil {
		return nil, errors.New("error ChainStore on GetTxReference")
	}
	reference, err := chainStore.GetTxReference(txn)
	if err != nil {
		return nil, err
	}
	data := make(map[*types.Input]*types.Output)
	for input, output := range reference {
		data[input] = output
	}
	return data, nil
}
