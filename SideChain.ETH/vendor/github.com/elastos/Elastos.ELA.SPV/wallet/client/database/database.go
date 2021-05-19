package database

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/wallet/store/headers"
	"github.com/elastos/Elastos.ELA.SPV/wallet/store/sqlite"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
)

func New(dataDir string) (*database, error) {
	dataStore, err := sqlite.NewDatabase(dataDir)
	if err != nil {
		return nil, err
	}

	return &database{
		lock:    new(sync.RWMutex),
		dataDir: dataDir,
		store:   dataStore,
	}, nil
}

type database struct {
	lock    *sync.RWMutex
	dataDir string
	store   sqlite.DataStore
}

func (d *database) AddAddress(address *common.Uint168, script []byte, addrType int) error {
	d.lock.Lock()
	defer d.lock.Unlock()

	return d.store.Addrs().Put(address, script, addrType)
}

func (d *database) GetAddress(address *common.Uint168) (*sutil.Addr, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.Addrs().Get(address)
}

func (d *database) GetAddrs() ([]*sutil.Addr, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.Addrs().GetAll()
}

func (d *database) DeleteAddress(address *common.Uint168) error {
	d.lock.Lock()
	defer d.lock.Unlock()

	return d.store.Addrs().Del(address)
}

func (d *database) GetAddressUTXOs(address *common.Uint168) ([]*sutil.UTXO, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.UTXOs().GetAddrAll(address)
}

func (d *database) GetAddressSTXOs(address *common.Uint168) ([]*sutil.STXO, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.STXOs().GetAddrAll(address)
}

func (d *database) BestHeight() uint32 {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.State().GetHeight()
}

func (d *database) Clear() error {
	d.lock.Lock()
	defer d.lock.Unlock()

	headers, err := headers.NewDatabase(d.dataDir)
	if err != nil {
		return err
	}

	err = headers.Clear()
	if err != nil {
		return err
	}

	err = d.store.Clear()
	if err != nil {
		return err
	}

	return nil
}
