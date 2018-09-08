package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

const CreateUTXOsDB = `CREATE TABLE IF NOT EXISTS UTXOs(
				OutPoint BLOB NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL,
				LockTime INTEGER NOT NULL,
				AtHeight INTEGER NOT NULL,
				ScriptHash BLOB NOT NULL
			);`

type UTXOs struct {
	*sync.RWMutex
	*sql.DB
}

func NewUTXOs(db *sql.DB, lock *sync.RWMutex) (*UTXOs, error) {
	_, err := db.Exec(CreateUTXOsDB)
	if err != nil {
		return nil, err
	}
	return &UTXOs{RWMutex: lock, DB: db}, nil
}

// put a utxo to database
func (db *UTXOs) Put(hash *common.Uint168, utxo *util.UTXO) error {
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
func (db *UTXOs) Get(outPoint *core.OutPoint) (*util.UTXO, error) {
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

	var value *common.Fixed64
	value, err = common.Fixed64FromBytes(valueBytes)
	if err != nil {
		return nil, err
	}

	return &util.UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight}, nil
}

// get utxos of the given script hash from database
func (db *UTXOs) GetAddrAll(hash *common.Uint168) ([]*util.UTXO, error) {
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

func (db *UTXOs) GetAll() ([]*util.UTXO, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT OutPoint, Value, LockTime, AtHeight FROM UTXOs")
	if err != nil {
		return []*util.UTXO{}, err
	}
	defer rows.Close()

	return db.getUTXOs(rows)
}

func (db *UTXOs) getUTXOs(rows *sql.Rows) ([]*util.UTXO, error) {
	var utxos []*util.UTXO
	for rows.Next() {
		var opBytes []byte
		var valueBytes []byte
		var lockTime uint32
		var atHeight uint32
		err := rows.Scan(&opBytes, &valueBytes, &lockTime, &atHeight)
		if err != nil {
			return utxos, err
		}

		outPoint, err := core.OutPointFromBytes(opBytes)
		if err != nil {
			return utxos, err
		}
		var value *common.Fixed64
		value, err = common.Fixed64FromBytes(valueBytes)
		if err != nil {
			return utxos, err
		}
		utxos = append(utxos, &util.UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight})
	}

	return utxos, nil
}

// delete a utxo from database
func (db *UTXOs) Delete(outPoint *core.OutPoint) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM UTXOs WHERE OutPoint=?", outPoint.Bytes())
	if err != nil {
		return err
	}

	return nil
}
