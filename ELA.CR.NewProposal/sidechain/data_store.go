package sidechain

import (
	"database/sql"
	"os"
	"sync"

	"github.com/elastos/Elastos.ELA/log"
	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName      = "sqlite3"
	DBName          = "./sideChainCache.db"
	QueryHeightCode = 0
)

const (
	CreateSideChainTxsTable = `CREATE TABLE IF NOT EXISTS SideChainTxs (
				TransactionHash VARCHAR NOT NULL PRIMARY KEY,
				GenesisBlockAddress VARCHAR(34)
			);`
)

var (
	DbCache DataStore
)

type DataStore interface {
	AddSideChainTx(transactionHash, genesisBlockAddress string) error
	HasSideChainTx(transactionHash string) (bool, error)

	ResetDataStore() error
}

type DataStoreImpl struct {
	sideMux   *sync.Mutex
	miningMux *sync.Mutex

	*sql.DB
}

func OpenDataStore() (DataStore, error) {
	db, err := initDB()
	if err != nil {
		return nil, err
	}
	dataStore := &DataStoreImpl{DB: db, sideMux: new(sync.Mutex), miningMux: new(sync.Mutex)}

	// Handle system interrupt signals
	dataStore.catchSystemSignals()

	return dataStore, nil
}

func initDB() (*sql.DB, error) {
	db, err := sql.Open(DriverName, DBName)
	if err != nil {
		log.Error("Open data db error:", err)
		return nil, err
	}
	// Create SideChainTxs table
	_, err = db.Exec(CreateSideChainTxsTable)
	if err != nil {
		return nil, err
	}

	return db, nil
}

func (store *DataStoreImpl) catchSystemSignals() {
	HandleSignal(func() {
		store.sideMux.Lock()
		store.miningMux.Lock()
		store.Close()
		os.Exit(-1)
	})
}

func (store *DataStoreImpl) ResetDataStore() error {

	store.DB.Close()
	os.Remove(DBName)

	var err error
	store.DB, err = initDB()
	if err != nil {
		return err
	}

	return nil
}

func (store *DataStoreImpl) AddSideChainTx(transactionHash, genesisBlockAddress string) error {
	store.sideMux.Lock()
	defer store.sideMux.Unlock()

	// Prepare sql statement
	stmt, err := store.Prepare("INSERT INTO SideChainTxs(TransactionHash, GenesisBlockAddress) values(?,?)")
	if err != nil {
		return err
	}
	// Do insert
	_, err = stmt.Exec(transactionHash, genesisBlockAddress)
	if err != nil {
		return err
	}
	return nil
}

func (store *DataStoreImpl) HasSideChainTx(transactionHash string) (bool, error) {
	store.sideMux.Lock()
	defer store.sideMux.Unlock()

	rows, err := store.Query(`SELECT GenesisBlockAddress FROM SideChainTxs WHERE TransactionHash=?`, transactionHash)
	defer rows.Close()
	if err != nil {
		return false, err
	}

	return rows.Next(), nil
}
