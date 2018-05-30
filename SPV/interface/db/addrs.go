package db

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/sdk"

	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/boltdb/bolt"
)

type Addrs interface {
	GetFilter() *sdk.AddrFilter
	Put(addr *common.Uint168) error
	GetAll() []*common.Uint168
}

var (
	BKTAddrs = []byte("Addrs")
)

type AddrStore struct {
	*sync.RWMutex
	*bolt.DB
	filter *sdk.AddrFilter
}

func NewAddrsDB(db *bolt.DB) (*AddrStore, error) {
	store := new(AddrStore)
	store.RWMutex = new(sync.RWMutex)
	store.DB = db

	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTAddrs)
		if err != nil {
			return err
		}
		return nil
	})

	addrs, err := store.getAll()
	if err != nil {
		return nil, err
	}
	store.filter = sdk.NewAddrFilter(addrs)

	return store, nil
}

func (a *AddrStore) GetFilter() *sdk.AddrFilter {
	a.Lock()
	defer a.Unlock()
	return a.filter
}

func (a *AddrStore) Put(addr *common.Uint168) error {
	a.Lock()
	defer a.Unlock()

	if a.filter.ContainAddr(*addr) {
		return nil
	}

	a.filter.AddAddr(addr)
	return a.Update(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTAddrs).Put(addr.Bytes(), addr.Bytes())
	})
}

func (a *AddrStore) GetAll() []*common.Uint168 {
	a.RLock()
	defer a.RUnlock()

	return a.filter.GetAddrs()
}

func (a *AddrStore) getAll() (addrs []*common.Uint168, err error) {
	err = a.View(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTAddrs).ForEach(func(k, v []byte) error {
			addr, err := common.Uint168FromBytes(v)
			if err != nil {
				return err
			}
			addrs = append(addrs, addr)
			return nil
		})
	})

	return addrs, err
}
