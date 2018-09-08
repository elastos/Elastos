package db

import (
	"database/sql"
	"fmt"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName = "sqlite3"
	DBName     = "./spv_wallet.db"
)

// Ensure SQLiteDB implement TxsDB interface.
var _ database.TxsDB = (*SQLiteDB)(nil)

type SQLiteDB struct {
	*sync.RWMutex
	*sql.DB

	state *StateDB
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

	// Create state db
	stateDB, err := NewStateDB(db, lock)
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

		state: stateDB,
		addrs: addrsDB,
		utxos: utxosDB,
		stxos: stxosDB,
		txs:   txnsDB,
	}, nil
}

func (db *SQLiteDB) Chain() Chain {
	return db.state
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

// checkDoubleSpends takes a transaction and compares it with
// all transactions in the db.  It returns a slice of all txIds in the db
// which are double spent by the received tx.
func (wallet *SQLiteDB) checkDoubleSpends(tx *util.Tx) ([]*common.Uint256, error) {
	var dubs []*common.Uint256
	txId := tx.Hash()
	txs, err := wallet.dataStore.Txs().GetAll()
	if err != nil {
		return nil, err
	}
	for _, compTx := range txs {
		// Skip coinbase transaction
		if compTx.Data.IsCoinBaseTx() {
			continue
		}
		// Skip duplicate transaction
		compTxId := compTx.Data.Hash()
		if compTxId.IsEqual(txId) {
			continue
		}
		for _, txIn := range tx.Inputs {
			for _, compIn := range compTx.Data.Inputs {
				if txIn.Previous.IsEqual(compIn.Previous) {
					// Found double spend
					dubs = append(dubs, &compTxId)
					break // back to txIn loop
				}
			}
		}
	}
	return dubs, nil
}

// Batch returns a TxBatch instance for transactions batch
// commit, this can get better performance when commit a bunch
// of transactions within a block.
func (db *SQLiteDB) Batch() database.TxBatch {

}

// CommitTx save a transaction into database, and return
// if it is a false positive and error.
func (db *SQLiteDB) CommitTx(tx *util.Tx) (bool, error) {
	txId := tx.Hash()
	height := tx.Height

	sh, ok := db.txIds.Get(txId)
	if ok && (sh > 0 || (sh == 0 && height == 0)) {
		return false, nil
	}

		dubs, err := db.checkDoubleSpends(tx)
		if err != nil {
			return false, nil
		}
		if len(dubs) > 0 {
			if height == 0 {
				return false, nil
			} else {
				// Rollback any double spend transactions
				for _, dub := range dubs {
					if err := db.RollbackTx(dub); err != nil {
						return false, nil
					}
				}
			}
		}

	hits := 0
	// Save UTXOs
	for index, output := range tx.Outputs {
		// Filter address
		if wallet.getAddrFilter().ContainAddr(output.ProgramHash) {
			var lockTime uint32
			if tx.TxType == core.CoinBase {
				lockTime = height + 100
			}
			utxo := ToUTXO(txId, height, index, output.Value, lockTime)
			err := wallet.dataStore.UTXOs().Put(&output.ProgramHash, utxo)
			if err != nil {
				return false, err
			}
			hits++
		}
	}

	// Put spent UTXOs to STXOs
	for _, input := range tx.Inputs {
		// Try to move UTXO to STXO, if a UTXO in database was spent, it will be moved to STXO
		err := wallet.dataStore.STXOs().FromUTXO(&input.Previous, &txId, height)
		if err == nil {
			hits++
		}
	}

	// If no hits, no need to save transaction
	if hits == 0 {
		return true, nil
	}

	// Save transaction
	err := wallet.dataStore.Txs().Put(db.NewTx(*tx, height))
	if err != nil {
		return false, err
	}

	wallet.txIds.Add(txId, height)

	return false, nil
}

// HaveTx returns if the transaction already saved in database
// by it's id.
func (db *SQLiteDB) HaveTx(txId *common.Uint256) (bool, error) {

}

// GetTxs returns all transactions within the given height.
func (db *SQLiteDB) GetTxs(height uint32) ([]*util.Tx, error) {

}

// RemoveTxs delete all transactions on the given height.  Return
// how many transactions are deleted from database.
func (db *SQLiteDB) RemoveTxs(height uint32) (int, error) {

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
		"SELECT OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight FROM STXOs WHERE SpendHeight=?", 0)
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

func (db *SQLiteDB) Clear() error {
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
