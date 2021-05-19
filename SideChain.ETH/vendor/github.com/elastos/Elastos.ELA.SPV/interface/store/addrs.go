package store

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/sdk"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/util"
)

var (
	BKTAddrs = []byte("A")
)

// Ensure addrs implement Addrs interface.
var _ Addrs = (*addrs)(nil)

type addrs struct {
	sync.RWMutex
	db     *leveldb.DB
	filter *sdk.AddrFilter
}

func NewAddrs(db *leveldb.DB) (*addrs, error) {
	store := addrs{db: db}

	addrs, err := store.getAll()
	if err != nil {
		return nil, err
	}
	store.filter = sdk.NewAddrFilter(addrs)

	return &store, nil
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
	return a.db.Put(toKey(BKTAddrs, addr[:]...), addr[:], nil)
}

func (a *addrs) GetAll() []*common.Uint168 {
	a.RLock()
	defer a.RUnlock()
	return a.filter.GetAddrs()
}

func (a *addrs) getAll() (addrs []*common.Uint168, err error) {
	it := a.db.NewIterator(util.BytesPrefix(BKTAddrs), nil)
	defer it.Release()
	for it.Next() {
		addr, err := common.Uint168FromBytes(it.Value())
		if err != nil {
			return nil, err
		}
		addrs = append(addrs, addr)
	}

	return addrs, it.Error()
}

func (a *addrs) Clear() error {
	a.Lock()
	defer a.Unlock()

	it := a.db.NewIterator(util.BytesPrefix(BKTAddrs), nil)
	defer it.Release()
	batch := new(leveldb.Batch)
	for it.Next() {
		batch.Delete(it.Key())
	}
	return a.db.Write(batch, nil)
}

func (a *addrs) Close() error {
	a.Lock()
	return nil
}
