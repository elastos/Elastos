package db

import (
	"database/sql"
	"fmt"
	"sync"

	"SPVWallet/bloom"

	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName = "sqlite3"
	DBName     = "./wallet.db"
)

var sqliteDb *SQLiteDB

type SQLiteDB struct {
	*sync.RWMutex
	*sql.DB

	scripts Scripts
	utxos   UTXOs
	stxos   STXOs
	txns    TXNs

	filterLock *sync.Mutex
}

func GetSQLiteDB() (DataStore, error) {
	if sqliteDb == nil {
		db, err := sql.Open(DriverName, DBName)
		if err != nil {
			fmt.Println("Open data db error:", err)
			return nil, err
		}
		// Use the save lock
		lock := new(sync.RWMutex)

		// Create scripts db
		scriptsDB, err := NewScriptDB(db, lock)
		if err != nil {
			return nil, err
		}
		// Create UTXOs db
		utxosDB, err := NewUTXOsDB(db, lock)
		if err != nil {
			return nil, err
		}
		// Create STXOs db
		stxosDB, err := NewSTXOsDB(db, lock)
		if err != nil {
			return nil, err
		}
		// Create TXNs db
		txnsDB, err := NewTXNsDB(db, lock)
		if err != nil {
			return nil, err
		}

		sqliteDb = &SQLiteDB{
			RWMutex: lock,
			DB:      db,

			scripts: scriptsDB,
			utxos:   utxosDB,
			stxos:   stxosDB,
			txns:    txnsDB,

			filterLock: new(sync.Mutex),
		}
	}

	return sqliteDb, nil
}

func (db *SQLiteDB) Scripts() Scripts {
	return db.scripts
}

func (db *SQLiteDB) UTXOs() UTXOs {
	return db.utxos
}

func (db *SQLiteDB) STXOs() STXOs {
	return db.stxos
}

func (db *SQLiteDB) TXNs() TXNs {
	return db.txns
}

func (db *SQLiteDB) GetFilter() *bloom.Filter {
	db.filterLock.Lock()
	defer db.filterLock.Unlock()

	addrs := db.scripts.GetFilter().GetScriptHashes()
	utxos, _ := db.utxos.GetAll()
	stxos, _ := db.stxos.GetAll()

	elements := uint32(len(addrs) + len(utxos) + len(stxos))
	filter := bloom.NewFilter(elements, 0, 0.00003)

	for _, addr := range addrs {
		filter.Add(addr.ToArray())
	}

	for _, utxo := range utxos {
		filter.AddOutPoint(&utxo.Op)
	}

	for _, stxo := range stxos {
		filter.AddOutPoint(&stxo.Op)
	}

	return filter
}

func (db *SQLiteDB) Close() {
	db.Lock()
	db.DB.Close()
}
