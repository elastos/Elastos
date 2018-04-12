package db

import (
	"database/sql"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/log"

	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName = "sqlite3"
	DBName     = "./spv_wallet.db"
)

type SQLiteDB struct {
	*sync.RWMutex
	*sql.DB

	info  Info
	addrs Addrs
	txs   Txs
	utxos UTXOs
	stxos STXOs
}

func NewSQLiteDB() (*SQLiteDB, error) {
	db, err := sql.Open(DriverName, DBName)
	if err != nil {
		fmt.Println("Open sqlite db error:", err)
		return nil, err
	}
	// Use the same lock
	lock := new(sync.RWMutex)

	// Create info db
	infoDB, err := NewInfoDB(db, lock)
	if err != nil {
		return nil, err
	}
	// Create addrs db
	addrsDB, err := NewAddrsDB(db, lock)
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
	// Create Txs db
	txnsDB, err := NewTxsDB(db, lock)
	if err != nil {
		return nil, err
	}

	return &SQLiteDB{
		RWMutex: lock,
		DB:      db,

		info:  infoDB,
		addrs: addrsDB,
		utxos: utxosDB,
		stxos: stxosDB,
		txs:   txnsDB,
	}, nil
}

func (db *SQLiteDB) Info() Info {
	return db.info
}

func (db *SQLiteDB) Addrs() Addrs {
	return db.addrs
}

func (db *SQLiteDB) Txs() Txs {
	return db.txs
}

func (db *SQLiteDB) UTXOs() UTXOs {
	return db.utxos
}

func (db *SQLiteDB) STXOs() STXOs {
	return db.stxos
}

func (db *SQLiteDB) Rollback(height uint32) error {
	db.Lock()
	defer db.Unlock()

	tx, err := db.Begin()
	if err != nil {
		return err
	}

	// Rollback UTXOs
	_, err = tx.Exec("DELETE FROM UTXOs WHERE AtHeight=?", height)
	if err != nil {
		return err
	}

	// Rollback STXOs, move UTXOs back first, then delete the STXOs
	_, err = tx.Exec(`INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, ScriptHash)
						SELECT OutPoint, Value, LockTime, AtHeight, ScriptHash FROM STXOs WHERE SpendHeight=?`, height)
	if err != nil {
		return err
	}
	_, err = tx.Exec("DELETE FROM STXOs WHERE SpendHeight=?", height)
	if err != nil {
		return err
	}

	// Rollback TXNs
	_, err = tx.Exec("DELETE FROM TXNs WHERE Height=?", height)
	if err != nil {
		return err
	}

	// Rollback Queue
	_, err = tx.Exec("DELETE FROM Queue WHERE Height=?", height)
	if err != nil {
		return err
	}

	return tx.Commit()
}

func (db *SQLiteDB) Reset() error {
	tx, err := db.Begin()
	if err != nil {
		return err
	}

	// Drop all tables except Addrs
	_, err = tx.Exec(`DROP TABLE IF EXISTS Info;
							DROP TABLE IF EXISTS UTXOs;
							DROP TABLE IF EXISTS STXOs;
							DROP TABLE IF EXISTS TXNs;
							DROP TABLE IF EXISTS Queue;`)
	if err != nil {
		return err
	}

	return tx.Commit()
}

func (db *SQLiteDB) Close() {
	db.Lock()
	db.DB.Close()
	log.Debug("SQLite DB closed")
}
