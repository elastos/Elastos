package mainchain

import (
	"database/sql"
	"os"
	"sync"

	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName      = "sqlite3"
	DBName          = "./mainChainCache.db"
	QueryHeightCode = 0
)

const (
	CreateMainChainTxsTable = `CREATE TABLE IF NOT EXISTS MainChainTxs (
				TransactionHash VARCHAR NOT NULL PRIMARY KEY
			);`
)

var (
	DbCache DataStore
)

type DataStore interface {
	AddMainChainTx(transactionHash string) error
	HasMainChainTx(transactionHash string) (bool, error)

	ResetDataStore() error
}

type DataStoreImpl struct {
	mainMux *sync.Mutex

	*sql.DB
}

func OpenDataStore() (DataStore, error) {
	db, err := initDB()
	if err != nil {
		return nil, err
	}
	dataStore := &DataStoreImpl{DB: db, mainMux: new(sync.Mutex)}

	// Handle system interrupt signals
	dataStore.catchSystemSignals()

	return dataStore, nil
}

func initDB() (*sql.DB, error) {
	db, err := sql.Open(DriverName, DBName)
	if err != nil {
		return nil, err
	}
	// Create SideChainTxs table
	_, err = db.Exec(CreateMainChainTxsTable)
	if err != nil {
		return nil, err
	}

	return db, nil
}

func (store *DataStoreImpl) catchSystemSignals() {
	HandleSignal(func() {
		store.mainMux.Lock()
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

func (store *DataStoreImpl) AddMainChainTx(transactionHash string) error {
	store.mainMux.Lock()
	defer store.mainMux.Unlock()

	// Prepare sql statement
	stmt, err := store.Prepare("INSERT INTO MainChainTxs(TransactionHash) values(?)")
	if err != nil {
		return err
	}
	// Do insert
	_, err = stmt.Exec(transactionHash)
	if err != nil {
		return err
	}
	return nil
}

func (store *DataStoreImpl) HasMainChainTx(transactionHash string) (bool, error) {
	store.mainMux.Lock()
	defer store.mainMux.Unlock()

	rows, err := store.Query(`SELECT TransactionHash FROM MainChainTxs WHERE TransactionHash=?`, transactionHash)
	defer rows.Close()
	if err != nil {
		return false, err
	}

	return rows.Next(), nil
}
