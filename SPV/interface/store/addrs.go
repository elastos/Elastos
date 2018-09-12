package store

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/sdk"

	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/boltdb/bolt"
)

var (
	BKTAddrs = []byte("Addrs")
)

// Ensure addrs implement Addrs interface.
var _ Addrs = (*addrs)(nil)

type addrs struct {
	*sync.RWMutex
	*bolt.DB
	filter *sdk.AddrFilter
}

func NewAddrs(db *bolt.DB) (*addrs, error) {
	store := new(addrs)
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

func (a *addrs) GetFilter() *sdk.AddrFilter {
	a.Lock()
	defer a.Unlock()
	return a.filter
}

func (a *addrs) Put(addr *common.Uint168) error {
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

func (a *addrs) GetAll() []*common.Uint168 {
	a.RLock()
	defer a.RUnlock()

	return a.filter.GetAddrs()
}

func (a *addrs) getAll() (addrs []*common.Uint168, err error) {
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

func (a *addrs) Clear() error {
	a.Lock()
	defer a.Unlock()
	return a.DB.Update(func(tx *bolt.Tx) error {
		return tx.DeleteBucket(BKTAddrs)
	})
}

func (a *addrs) Close() error {
	a.Lock()
	return nil
}
