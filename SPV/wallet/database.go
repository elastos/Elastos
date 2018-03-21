package wallet

import (
	"sync"

	. "SPVWallet/core"
	. "SPVWallet/db"
)

type Database interface {
	AddAddress(address *Uint168, script []byte) error
	GetAddress(address *Uint168) (*Addr, error)
	GetAddrs() ([]*Addr, error)
	DeleteAddress(address *Uint168) error
	GetAddressUTXOs(address *Uint168) ([]*UTXO, error)
	GetAddressSTXOs(address *Uint168) ([]*STXO, error)
	GetTxns() ([]*Txn, error)
	ChainHeight() uint32
	Reset() error
}

var instance Database

func GetDatabase() (Database, error) {
	if instance == nil {
		dataStore, err := NewSQLiteDB()
		if err != nil {
			return nil, err
		}

		instance = &DatabaseImpl{
			lock:      new(sync.RWMutex),
			DataStore: dataStore,
		}
	}

	return instance, nil
}

type DatabaseImpl struct {
	lock *sync.RWMutex
	DataStore

	filter *AddrFilter
}

func (db *DatabaseImpl) AddAddress(address *Uint168, script []byte) error {
	db.lock.Lock()
	defer db.lock.Unlock()

	return db.DataStore.Addrs().Put(address, script)
}

func (db *DatabaseImpl) GetAddress(address *Uint168) (*Addr, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.Addrs().Get(address)
}

func (db *DatabaseImpl) GetAddrs() ([]*Addr, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.Addrs().GetAll()
}

func (db *DatabaseImpl) DeleteAddress(address *Uint168) error {
	db.lock.Lock()
	defer db.lock.Unlock()

	return db.DataStore.Addrs().Delete(address)
}

func (db *DatabaseImpl) GetAddressUTXOs(address *Uint168) ([]*UTXO, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.UTXOs().GetAddrAll(address)
}

func (db *DatabaseImpl) GetAddressSTXOs(address *Uint168) ([]*STXO, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.STXOs().GetAddrAll(address)
}

func (db *DatabaseImpl) GetTxns() ([]*Txn, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.TXNs().GetAll()
}

func (db *DatabaseImpl) ChainHeight() uint32 {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.Info().ChainHeight()
}

func (db *DatabaseImpl) Reset() error {
	db.lock.Lock()
	defer db.lock.Unlock()

	headers, err := NewHeadersDB()
	if err != nil {
		return err
	}

	for {
		header, err := headers.Rollback()
		if err != nil {
			return err
		}
		err = db.DataStore.Rollback(header.Height)
		if err != nil {
			return err
		}
		if header.Height == 0 {
			break
		}
	}

	return nil
}
