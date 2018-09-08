package database

import (
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/store"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/store/sqlite"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Database interface {
	AddAddress(address *common.Uint168, script []byte, addrType int) error
	GetAddress(address *common.Uint168) (*util.Addr, error)
	GetAddrs() ([]*util.Addr, error)
	DeleteAddress(address *common.Uint168) error
	GetAddressUTXOs(address *common.Uint168) ([]*util.UTXO, error)
	GetAddressSTXOs(address *common.Uint168) ([]*util.STXO, error)
	BestHeight() uint32
	Clear() error
}

func New() (Database, error) {
	dataStore, err := sqlite.New()
	if err != nil {
		return nil, err
	}

	return &database{
		lock:  new(sync.RWMutex),
		store: dataStore,
	}, nil
}

type database struct {
	lock  *sync.RWMutex
	store store.DataStore
}

func (d *database) AddAddress(address *common.Uint168, script []byte, addrType int) error {
	d.lock.Lock()
	defer d.lock.Unlock()

	return d.store.Addrs().Put(address, script, addrType)
}

func (d *database) GetAddress(address *common.Uint168) (*util.Addr, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.Addrs().Get(address)
}

func (d *database) GetAddrs() ([]*util.Addr, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.Addrs().GetAll()
}

func (d *database) DeleteAddress(address *common.Uint168) error {
	d.lock.Lock()
	defer d.lock.Unlock()

	return d.store.Addrs().Delete(address)
}

func (d *database) GetAddressUTXOs(address *common.Uint168) ([]*util.UTXO, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.UTXOs().GetAddrAll(address)
}

func (d *database) GetAddressSTXOs(address *common.Uint168) ([]*util.STXO, error) {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.STXOs().GetAddrAll(address)
}

func (d *database) BestHeight() uint32 {
	d.lock.RLock()
	defer d.lock.RUnlock()

	return d.store.Chain().GetHeight()
}

func (d *database) Clear() error {
	d.lock.Lock()
	defer d.lock.Unlock()

	headers, err := database.NewHeadersDB()
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
