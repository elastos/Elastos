package db

import (
	"database/sql"
	"sync"

	. "github.com/elastos/Elastos.ELA.Utility/common"
	. "github.com/elastos/Elastos.ELA/core"
)

const CreateUTXOsDB = `CREATE TABLE IF NOT EXISTS UTXOs(
				OutPoint BLOB NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL,
				LockTime INTEGER NOT NULL,
				AtHeight INTEGER NOT NULL,
				ScriptHash BLOB NOT NULL
			);`

type UTXOsDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewUTXOsDB(db *sql.DB, lock *sync.RWMutex) (UTXOs, error) {
	_, err := db.Exec(CreateUTXOsDB)
	if err != nil {
		return nil, err
	}
	return &UTXOsDB{RWMutex: lock, DB: db}, nil
}

// put a utxo to database
func (db *UTXOsDB) Put(hash *Uint168, utxo *UTXO) error {
	db.Lock()
	defer db.Unlock()

	valueBytes, err := utxo.Value.Bytes()
	if err != nil {
		return err
	}
	sql := "INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, ScriptHash) VALUES(?,?,?,?,?)"
	_, err = db.Exec(sql, utxo.Op.Bytes(), valueBytes, utxo.LockTime, utxo.AtHeight, hash.Bytes())
	if err != nil {
		return err
	}

	return nil
}

// get a utxo from database
func (db *UTXOsDB) Get(outPoint *OutPoint) (*UTXO, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Value, LockTime, AtHeight FROM UTXOs WHERE OutPoint=?`, outPoint.Bytes())
	var valueBytes []byte
	var lockTime uint32
	var atHeight uint32
	err := row.Scan(&valueBytes, &lockTime, &atHeight)
	if err != nil {
		return nil, err
	}

	var value *Fixed64
	value, err = Fixed64FromBytes(valueBytes)
	if err != nil {
		return nil, err
	}

	return &UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight}, nil
}

// get utxos of the given script hash from database
func (db *UTXOsDB) GetAddrAll(hash *Uint168) ([]*UTXO, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query(
		"SELECT OutPoint, Value, LockTime, AtHeight FROM UTXOs WHERE ScriptHash=?", hash.Bytes())
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return db.getUTXOs(rows)
}

func (db *UTXOsDB) GetAll() ([]*UTXO, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT OutPoint, Value, LockTime, AtHeight FROM UTXOs")
	if err != nil {
		return []*UTXO{}, err
	}
	defer rows.Close()

	return db.getUTXOs(rows)
}

func (db *UTXOsDB) getUTXOs(rows *sql.Rows) ([]*UTXO, error) {
	var utxos []*UTXO
	for rows.Next() {
		var opBytes []byte
		var valueBytes []byte
		var lockTime uint32
		var atHeight uint32
		err := rows.Scan(&opBytes, &valueBytes, &lockTime, &atHeight)
		if err != nil {
			return utxos, err
		}

		outPoint, err := OutPointFromBytes(opBytes)
		if err != nil {
			return utxos, err
		}
		var value *Fixed64
		value, err = Fixed64FromBytes(valueBytes)
		if err != nil {
			return utxos, err
		}
		utxos = append(utxos, &UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight})
	}

	return utxos, nil
}

// delete a utxo from database
func (db *UTXOsDB) Delete(outPoint *OutPoint) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM UTXOs WHERE OutPoint=?", outPoint.Bytes())
	if err != nil {
		return err
	}

	return nil
}
