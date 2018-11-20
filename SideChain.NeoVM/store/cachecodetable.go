package store
//
//import (
//	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/states"
//	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
//
//	"github.com/elastos/Elastos.ELA.SideChain/types"
//	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/storage"
//)
//
//type CacheCodeTable struct {
//	dbCache *storage.DBCache
//}
//
//func NewCacheCodeTable(dbCache *storage.DBCache) *CacheCodeTable {
//	return &CacheCodeTable{dbCache: dbCache}
//}
//
//func (table *CacheCodeTable) GetScript(codeHash []byte) ([]byte) {
//	value, err := table.dbCache.TryGet(ST_Contract, string(codeHash))
//	if err != nil {
//		return nil
//	}
//	return value.(*states.ContractState).Code.Code
//}
//
//func (table *CacheCodeTable) GetTxReference(tx *interfaces.IDataContainer) (map[*types.Input]*types.Output, error) {
//	txn := (*tx).(*types.Transaction)
//	store := table.dbCache.GetChainStoreDb()
//	reference, err := store.GetTxReference(txn)
//	if err != nil {
//		return nil, err
//	}
//	data := make(map[*types.Input]*types.Output)
//	for input, output := range reference {
//		data[input] = output
//	}
//	return data, nil
//}