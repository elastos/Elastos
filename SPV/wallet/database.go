package wallet

import (
	"sync"

	. "SPVWallet/core"
	. "SPVWallet/db"
)

type Database interface {
	AddAddress(address *Uint168, script []byte) error
	GetScript(address *Uint168) ([]byte, error)
	GetScripts() ([][]byte, error)
	DeleteAddress(address *Uint168) error
	GetAddressUTXOs(address *Uint168) ([]*UTXO, error)
	GetAddressSTXOs(address *Uint168) ([]*STXO, error)
	GetTxns() ([]*Txn, error)
	Reset() error
}

var instance Database

func GetDatabase() (Database, error) {
	if instance == nil {
		headers, err := NewHeadersDB()
		if err != nil {
			return nil, err
		}

		dataStore, err := NewSQLiteDB()
		if err != nil {
			return nil, err
		}

		instance = &DatabaseImpl{
			lock:      new(sync.RWMutex),
			Headers:   headers,
			DataStore: dataStore,
		}
	}

	return instance, nil
}

type DatabaseImpl struct {
	lock *sync.RWMutex
	Headers
	DataStore

	filter *ScriptFilter
}

func (db *DatabaseImpl) AddAddress(address *Uint168, script []byte) error {
	db.lock.Lock()
	defer db.lock.Unlock()

	return db.DataStore.Scripts().Put(address, script)
}

func (db *DatabaseImpl) GetScript(address *Uint168) ([]byte, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.Scripts().Get(address)
}

func (db *DatabaseImpl) GetScripts() ([][]byte, error) {
	db.lock.RLock()
	defer db.lock.RUnlock()

	return db.DataStore.Scripts().GetAll()
}

func (db *DatabaseImpl) DeleteAddress(address *Uint168) error {
	db.lock.Lock()
	defer db.lock.Unlock()

	return db.DataStore.Scripts().Delete(address)
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

func (db *DatabaseImpl) Reset() error {
	db.lock.Lock()
	defer db.lock.Unlock()

	for {
		header, err := db.Headers.Rollback()
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
