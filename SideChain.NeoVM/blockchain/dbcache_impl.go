package blockchain

import (
	"bytes"

	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/database"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/storage"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/contract/states"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/smartcontract/enumerators"
)

type DBCache struct {
	RWSet *storage.RWSet
	db    database.Database
}

func NewDBCache(db database.Database) *DBCache {
	return &DBCache{
		RWSet: storage.NewRWSet(),
		db:    db,
	}
}

func (cache *DBCache) Commit() {
	rwSet := cache.RWSet.WriteSet
	for k, v := range rwSet {
		key := make([]byte, 0)
		key = append([]byte{byte(v.Prefix)}, []byte(k)...)
		if v.IsDeleted {
			cache.db.Delete(key)
		} else {
			b := new(bytes.Buffer)
			v.Item.Serialize(b)
			value := make([]byte, 0)
			value = append(value, b.Bytes()...)
			cache.db.Put(key, value)
		}
	}
}

func (cache *DBCache) TryGetInternal(prefix blockchain.EntryPrefix, key string) (states.IStateValueInterface, error) {
	k := make([]byte, 0)
	k = append([]byte{byte(prefix)}, []byte(key)...)
	value, err := cache.db.Get(k)
	if err != nil {
		return nil, err
	}
	return states.GetStateValue(prefix, value)
}

func (cache *DBCache) TryDelete(prefix blockchain.EntryPrefix, key string) bool {
	k := make([]byte, 0)
	k = append([]byte{byte(prefix)}, []byte(key)...)
	err := cache.db.Delete(k)
	return err == nil
}

func (cache *DBCache) GetOrAdd(prefix blockchain.EntryPrefix, key string, value states.IStateValueInterface) (states.IStateValueInterface, error) {
	if v, ok := cache.RWSet.WriteSet[key]; ok {
		if v.IsDeleted {
			v.Item = value
			v.IsDeleted = false
		}
	} else {
		item, err := cache.TryGetInternal(prefix, key)
		if err != nil && err.Error() != ("leveldb: not found") {
			return nil, err
		}
		write := &storage.Write{
			Prefix:    prefix,
			Key:       key,
			Item:      item,
			IsDeleted: false,
		}
		if write.Item == nil {
			write.Item = value
		}
		cache.RWSet.WriteSet[key] = write
	}
	return cache.RWSet.WriteSet[key].Item, nil
}

func (cache *DBCache) TryGet(prefix blockchain.EntryPrefix, key string) (states.IStateValueInterface, error) {
	if v, ok := cache.RWSet.WriteSet[key]; ok {
		return v.Item, nil
	} else {
		return cache.TryGetInternal(prefix, key)
	}
}

func (cache *DBCache) GetWriteSet() *storage.RWSet {
	return cache.RWSet
}

func (cache *DBCache) GetChainStoreDb() database.Database {
	return cache.db
}

func (cache *DBCache) FindInternal(prefix blockchain.EntryPrefix, keyPrefix string) database.Iterator {
	k := make([]byte, 0)
	k = append([]byte{byte(prefix)}, []byte(keyPrefix)...)
	count := len(k)
	iteratorList := enumerators.NewListIterator()
	for key, v := range cache.RWSet.WriteSet {
		kvalue := key[0 : count - 1]
		if kvalue == keyPrefix {
			ky := append([]byte{byte(prefix)}, []byte(key)...)
			iteratorList.Add(ky, v.Item)
		}
	}
	if iteratorList.HashNext() {
		return iteratorList
	}

	iter := cache.db.NewIterator(k)
	return enumerators.NewIterator(iter)
}