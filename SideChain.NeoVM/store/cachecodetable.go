package store

import (
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain/types"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

type CacheCodeTable struct {
	dbCache *DBCache
}

func NewCacheCodeTable(dbCache *DBCache) *CacheCodeTable {
	return &CacheCodeTable{dbCache: dbCache}
}

func (table *CacheCodeTable) GetScript(codeHash []byte) ([]byte) {
	value, err := table.dbCache.TryGet(states.ST_Contract, string(codeHash))
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
