package db

import (
	"bytes"
	"time"
	"database/sql"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA/core"

	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName = "sqlite3"
	DBName     = "./spv_wallet.db"
)

type SQLiteDB struct {
	*sync.RWMutex
	*sql.DB

	chain Chain
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

	// Create chain db
	chainDB, err := NewChainDB(db, lock)
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

		chain: chainDB,
		addrs: addrsDB,
		utxos: utxosDB,
		stxos: stxosDB,
		txs:   txnsDB,
	}, nil
}

func (db *SQLiteDB) Chain() Chain {
	return db.chain
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

	return tx.Commit()
}

func (db *SQLiteDB) RollbackTimeoutTxs(timeoutDuration time.Duration) error {
	log.Debug()
	db.Lock()
	defer db.Unlock()

	tx, err := db.Begin()
	if err != nil {
		return err
	}

	// Get timeout transactions
	timeoutTime := time.Now().Add(-timeoutDuration).Unix()
	rows, err := tx.Query("SELECT RawData FROM TXNs WHERE Height=? AND Timestamp<?", 0, timeoutTime)
	if err != nil {
		return err
	}
	for rows.Next() {
		var txRawData []byte
		rows.Scan(&txRawData)
		var transaction core.Transaction
		err = transaction.DeserializeUnsigned(bytes.NewReader(txRawData))
		if err != nil {
			return err
		}

		log.Debugf("Timeout transaction %v", transaction.String())

		// Rollback STXOs
		for _, input := range transaction.Inputs {
			_, err = tx.Exec(`INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, ScriptHash)
			SELECT OutPoint, Value, LockTime, AtHeight, ScriptHash FROM STXOs WHERE OutPoint=?`, input.Previous.Bytes())
			if err != nil {
				return err
			}
			log.Debugf("Input %v rollback", input)
		}

		// Remove UTXOs
		for index := range transaction.Outputs {
			outpoint := core.NewOutPoint(transaction.Hash(), uint16(index))
			_, err = tx.Exec("DELETE FROM UTXOs WHERE OutPoint=?", outpoint.Bytes())
			if err != nil {
				return err
			}
			log.Debugf("Output index %d rollback", index)
		}
	}

	return tx.Commit()
}

func (db *SQLiteDB) Reset() error {
	tx, err := db.Begin()
	if err != nil {
		return err
	}

	// Drop all tables except Addrs
	_, err = tx.Exec(`DROP TABLE IF EXISTS Chain;
							DROP TABLE IF EXISTS UTXOs;
							DROP TABLE IF EXISTS STXOs;
							DROP TABLE IF EXISTS TXNs;`)
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
