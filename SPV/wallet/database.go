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
}

var instance Database

func GetDatabase() (Database, error) {
	if instance == nil {
		dataStore, err := GetSQLiteDB()
		if err != nil {
			return nil, err
		}

		instance = &DatabaseImpl{
			DataStore: dataStore,
		}
	}

	return instance, nil
}

type DatabaseImpl struct {
	sync.RWMutex
	Headers
	DataStore

	filter *ScriptFilter
}

func (db *DatabaseImpl) AddAddress(address *Uint168, script []byte) error {
	db.Lock()
	defer db.Unlock()

	return db.DataStore.Scripts().Put(address, script)
}

func (db *DatabaseImpl) GetScript(address *Uint168) ([]byte, error) {
	db.RLock()
	defer db.RUnlock()

	return db.DataStore.Scripts().Get(address)
}

func (db *DatabaseImpl) GetScripts() ([][]byte, error) {
	db.RLock()
	defer db.RUnlock()

	return db.DataStore.Scripts().GetAll()
}

func (db *DatabaseImpl) DeleteAddress(address *Uint168) error {
	db.Lock()
	defer db.Unlock()

	return db.DataStore.Scripts().Delete(address)
}

func (db *DatabaseImpl) GetAddressUTXOs(address *Uint168) ([]*UTXO, error) {
	db.RLock()
	defer db.RUnlock()

	return db.DataStore.UTXOs().GetAddrAll(address)
}

func (db *DatabaseImpl) GetAddressSTXOs(address *Uint168) ([]*STXO, error) {
	db.RLock()
	defer db.RUnlock()

	return db.DataStore.STXOs().GetAddrAll(address)
}

func (db *DatabaseImpl) GetTxns() ([]*Txn, error) {
	db.RLock()
	defer db.RUnlock()

	return db.DataStore.TXNs().GetAll()
}
