package db

import (
	"database/sql"
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

const CreateTXNDB = `CREATE TABLE IF NOT EXISTS TXNs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Height INTEGER NOT NULL,
				RawData BLOB NOT NULL
			);`

type TXNsDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewTXNsDB(db *sql.DB, lock *sync.RWMutex) (TXNs, error) {
	_, err := db.Exec(CreateTXNDB)
	if err != nil {
		return nil, err
	}
	return &TXNsDB{RWMutex: lock, DB: db}, nil
}

// Put a new transaction to database
func (db *TXNsDB) Put(txn *Txn) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare(`INSERT OR REPLACE INTO TXNs(Hash, Height, RawData) VALUES(?,?,?)`)
	if err != nil {
		return err
	}

	_, err = stmt.Exec(txn.TxId.Bytes(), txn.Height, txn.RawData)
	if err != nil {
		return err
	}

	return nil
}

// Fetch a raw tx and it's metadata given a hash
func (db *TXNsDB) Get(txId *Uint256) (*Txn, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Height, RawData FROM TXNs WHERE Hash=?`, txId.Bytes())
	var height uint32
	var rawData []byte
	err := row.Scan(&height, &rawData)
	if err != nil {
		return nil, err
	}

	return &Txn{TxId: *txId, Height: height, RawData: rawData}, nil
}

// Fetch all transactions from database
func (db *TXNsDB) GetAll() ([]*Txn, error) {
	db.RLock()
	defer db.RUnlock()

	var txns []*Txn
	rows, err := db.Query("SELECT Hash, Height, RawData FROM TXNs")
	if err != nil {
		return txns, err
	}
	defer rows.Close()

	for rows.Next() {
		var txIdBytes []byte
		var height uint32
		var rawData []byte
		err := rows.Scan(&txIdBytes, &height, &rawData)
		if err != nil {
			return txns, err
		}

		txId, err := Uint256FromBytes(txIdBytes)
		if err != nil {
			return txns, err
		}
		txns = append(txns, &Txn{TxId: *txId, Height: height, RawData: rawData})
	}

	return txns, nil
}

// Fetch all transactions from the given height
func (db *TXNsDB) GetAllFrom(height uint32) ([]*Txn, error) {
	db.RLock()
	defer db.RUnlock()

	var txns []*Txn
	rows, err := db.Query("SELECT Hash, Height, RawData FROM TXNs WHERE Height=?", height)
	if err != nil {
		return txns, err
	}
	defer rows.Close()

	for rows.Next() {
		var txIdBytes []byte
		var height uint32
		var rawData []byte
		err := rows.Scan(&txIdBytes, &height, &rawData)
		if err != nil {
			return txns, err
		}

		txId, err := Uint256FromBytes(txIdBytes)
		if err != nil {
			return txns, err
		}
		txns = append(txns, &Txn{TxId: *txId, Height: height, RawData: rawData})
	}

	return txns, nil
}

// Update the height of a transaction
func (db *TXNsDB) UpdateHeight(txId *Uint256, height uint32) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("UPDATE TXNs SET Height=? WHERE Hash=?")
	if err != nil {
		return err
	}

	_, err = stmt.Exec(height, txId.Bytes())
	if err != nil {
		return err
	}

	return nil
}

// Delete a transaction from the db
func (db *TXNsDB) Delete(txId *Uint256) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("DELETE FROM TXNs WHERE Hash=?")
	if err != nil {
		return err
	}

	_, err = stmt.Exec(txId.Bytes())
	if err != nil {
		return err
	}

	return nil
}
