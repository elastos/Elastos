package storage

import (
	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	nc "github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
)

type DBCache interface {
	GetOrAdd(prefix blockchain.EntryPrefix, key string, value states.IStateValueInterface) (states.IStateValueInterface, error)
	TryGet(prefix blockchain.EntryPrefix, key string) (states.IStateValueInterface, error)
	TryDelete(prefix blockchain.EntryPrefix, key string) bool
	GetWriteSet() *RWSet
	FindInternal(prefix blockchain.EntryPrefix, keyPrefix string) database.Iterator
}

type CloneCache struct {
	innerCache DBCache
	dbCache    DBCache
}

func NewCloneDBCache(innerCache DBCache, dbCache DBCache) *CloneCache {
	return &CloneCache{
		innerCache: innerCache,
		dbCache:    dbCache,
	}
}

func (cloneCache *CloneCache) GetInnerCache() DBCache {
	return cloneCache.innerCache
}

func (cloneCache *CloneCache) Commit() {
	for _, v := range cloneCache.innerCache.GetWriteSet().WriteSet {
		if v.IsDeleted {
			cloneCache.innerCache.GetWriteSet().Delete(v.Prefix, v.Key)
		} else {
			cloneCache.innerCache.GetWriteSet().Add(v.Prefix, v.Key, v.Item)
		}
	}
}

func (cloneCache *CloneCache) TryGet(prefix blockchain.EntryPrefix, key string) (states.IStateValueInterface, error) {
	if v, ok := cloneCache.innerCache.GetWriteSet().WriteSet[key]; ok {
		return v.Item, nil
	} else {
		return cloneCache.dbCache.TryGet(prefix, key)
	}
}

func (cloneCache *CloneCache) TryDelete(prefix blockchain.EntryPrefix, hash common.Uint168) bool {
	keyStr := string(nc.UInt168ToUInt160(&hash))
	cloneCache.innerCache.GetWriteSet().Delete(prefix, keyStr)
	result := cloneCache.dbCache.TryDelete(prefix, keyStr)
	return result
}

func (cloneCache* CloneCache) Find(prefix blockchain.EntryPrefix, keyPreFix string) database.Iterator {
	iterator := cloneCache.innerCache.FindInternal(prefix, keyPreFix)
	return iterator
}