package sqlite

import (
	"database/sql"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"
	"sync"
	"fmt"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

const CreateSTXOsDB = `CREATE TABLE IF NOT EXISTS STXOs(
				OutPoint BLOB NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL,
				LockTime INTEGER NOT NULL,
				AtHeight INTEGER NOT NULL,
				SpendHash BLOB NOT NULL,
				SpendHeight INTEGER NOT NULL,
				ScriptHash BLOB NOT NULL
			);`

type STXOs struct {
	*sync.RWMutex
	*sql.DB
}

func NewSTXOs(db *sql.DB, lock *sync.RWMutex) (*STXOs, error) {
	_, err := db.Exec(CreateSTXOsDB)
	if err != nil {
		return nil, err
	}
	return &STXOs{RWMutex: lock, DB: db}, nil
}

// Move a UTXO to STXO
func (db *STXOs) FromUTXO(outPoint *core.OutPoint, spendTxId *common.Uint256, spendHeight uint32) error {
	db.Lock()
	defer db.Unlock()

	tx, err := db.Begin()
	if err != nil {
		return err
	}
	defer tx.Rollback()

	sql := `INSERT OR REPLACE INTO STXOs(OutPoint, Value, LockTime, AtHeight, ScriptHash, SpendHash, SpendHeight)
			SELECT UTXOs.OutPoint, UTXOs.Value, UTXOs.LockTime, UTXOs.AtHeight, UTXOs.ScriptHash, ?, ? FROM UTXOs
			WHERE OutPoint=?`
	result, err := tx.Exec(sql, spendTxId.Bytes(), spendHeight, outPoint.Bytes())
	if err != nil {
		return err
	}

	rows, err := result.RowsAffected()
	if err != nil {
		return nil
	}

	if rows == 0 {
		return fmt.Errorf("no rows effected")
	}

	_, err = tx.Exec("DELETE FROM UTXOs WHERE OutPoint=?", outPoint.Bytes())
	if err != nil {
		return err
	}

	return tx.Commit()
}

// get a stxo from database
func (db *STXOs) Get(outPoint *core.OutPoint) (*util.STXO, error) {
	db.RLock()
	defer db.RUnlock()

	sql := `SELECT Value, LockTime, AtHeight, SpendHash, SpendHeight FROM STXOs WHERE OutPoint=?`
	row := db.QueryRow(sql, outPoint.Bytes())
	var valueBytes []byte
	var lockTime uint32
	var atHeight uint32
	var spendHashBytes []byte
	var spendHeight uint32
	err := row.Scan(&valueBytes, &lockTime, &atHeight, &spendHashBytes, &spendHeight)
	if err != nil {
		return nil, err
	}

	var value *common.Fixed64
	value, err = common.Fixed64FromBytes(valueBytes)
	if err != nil {
		return nil, err
	}

	var utxo = util.UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight}
	spendHash, err := common.Uint256FromBytes(spendHashBytes)
	if err != nil {
		return nil, err
	}

	return &util.STXO{UTXO: utxo, SpendTxId: *spendHash, SpendHeight: spendHeight}, nil
}

// get stxos of the given script hash from database
func (db *STXOs) GetAddrAll(hash *common.Uint168) ([]*util.STXO, error) {
	db.RLock()
	defer db.RUnlock()

	sql := "SELECT OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight FROM STXOs WHERE ScriptHash=?"
	rows, err := db.Query(sql, hash.Bytes())
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return db.getSTXOs(rows)
}

func (db *STXOs) GetAll() ([]*util.STXO, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight FROM STXOs")
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return db.getSTXOs(rows)
}

func (db *STXOs) getSTXOs(rows *sql.Rows) ([]*util.STXO, error) {
	var stxos []*util.STXO
	for rows.Next() {
		var opBytes []byte
		var valueBytes []byte
		var lockTime uint32
		var atHeight uint32
		var spendHashBytes []byte
		var spendHeight uint32
		err := rows.Scan(&opBytes, &valueBytes, &lockTime, &atHeight, &spendHashBytes, &spendHeight)
		if err != nil {
			return stxos, err
		}

		outPoint, err := core.OutPointFromBytes(opBytes)
		if err != nil {
			return stxos, err
		}
		var value *common.Fixed64
		value, err = common.Fixed64FromBytes(valueBytes)
		if err != nil {
			return stxos, err
		}
		var utxo = util.UTXO{Op: *outPoint, Value: *value, LockTime: lockTime, AtHeight: atHeight}
		spendHash, err := common.Uint256FromBytes(spendHashBytes)
		if err != nil {
			return stxos, err
		}

		stxos = append(stxos, &util.STXO{UTXO: utxo, SpendTxId: *spendHash, SpendHeight: spendHeight})
	}

	return stxos, nil
}

// delete a stxo from database
func (db *STXOs) Delete(outPoint *core.OutPoint) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM STXOs WHERE OutPoint=?", outPoint.Bytes())
	if err != nil {
		return err
	}

	return nil
}
