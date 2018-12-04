package db

import (
	"database/sql"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/log"
	_ "github.com/mattn/go-sqlite3"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	DriverName = "sqlite3"
	DBName     = "./spv_wallet.db"
)

type SQLiteDB struct {
	*sync.RWMutex
	*sql.DB

	chain *ChainDB
	addrs *AddrsDB
	txs   *TxsDB
	utxos *UTXOsDB
	stxos *STXOsDB
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
	defer tx.Rollback()

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

func (db *SQLiteDB) RollbackTx(txId *common.Uint256) error {
	db.Lock()
	defer db.Unlock()

	return db.rollbackTx(txId)
}

func (db *SQLiteDB) rollbackTx(txId *common.Uint256) error {
	tx, err := db.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	// Get unconfirmed STXOs
	rows, err := db.Query(
		"SELECT OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight FROM STXOs WHERE SpendHeight=?",0)
	if err != nil {
		return err
	}
	defer rows.Close()

	stxos, err := db.stxos.getSTXOs(rows)
	if err != nil {
		return err
	}

	for _, stxo := range stxos {
		outpoint := stxo.Op.Bytes()
		if txId.IsEqual(stxo.SpendTxId) {
			// Restore UTXO
			_, err = tx.Exec(`INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, ScriptHash)
			SELECT OutPoint, Value, LockTime, AtHeight, ScriptHash FROM STXOs WHERE OutPoint=?`, outpoint)
			if err != nil {
				return err
			}
			// Delele STXO
			_, err = tx.Exec("DELETE FROM STXOs WHERE OutPoint=?", outpoint)
			if err != nil {
				return err
			}
		}
		if txId.IsEqual(stxo.UTXO.Op.TxID) {
			// Delele STXO
			_, err = tx.Exec("DELETE FROM STXOs WHERE OutPoint=?", outpoint)
			if err != nil {
				return err
			}
			if err := db.rollbackTx(&stxo.SpendTxId); err != nil {
				return err
			}
		}
	}
	// Get unconfirmed UTXOs
	rows, err = db.Query("SELECT OutPoint, Value, LockTime, AtHeight FROM UTXOs WHERE AtHeight=?", 0)
	if err != nil {
		return err
	}
	defer rows.Close()

	utxos, err := db.utxos.getUTXOs(rows)
	if err != nil {
		return err
	}

	for _, utxo := range utxos {
		if txId.IsEqual(utxo.Op.TxID) {
			// Delele UTXO
			_, err = tx.Exec("DELETE FROM UTXOs WHERE OutPoint=?", utxo.Op.Bytes())
			if err != nil {
				return err
			}
		}
	}

	// Delele transaction
	_, err = tx.Exec("DELETE FROM TXNs WHERE Hash=?", txId.Bytes())
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
