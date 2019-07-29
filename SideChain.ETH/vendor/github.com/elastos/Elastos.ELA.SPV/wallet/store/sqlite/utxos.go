package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
)

const CreateUTXOsDB = `CREATE TABLE IF NOT EXISTS UTXOs(
				OutPoint BLOB NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL,
				LockTime INTEGER NOT NULL,
				AtHeight INTEGER NOT NULL,
				Address BLOB NOT NULL
			);`

// Ensure utxos implement UTXOs interface.
var _ UTXOs = (*utxos)(nil)

type utxos struct {
	*sync.RWMutex
	*sql.DB
}

func NewUTXOs(db *sql.DB, lock *sync.RWMutex) (*utxos, error) {
	_, err := db.Exec(CreateUTXOsDB)
	if err != nil {
		return nil, err
	}
	return &utxos{RWMutex: lock, DB: db}, nil
}

// put a utxo to database
func (u *utxos) Put(utxo *sutil.UTXO) error {
	u.Lock()
	defer u.Unlock()

	valueBytes, err := utxo.Value.Bytes()
	if err != nil {
		return err
	}
	sql := "INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, Address) VALUES(?,?,?,?,?)"
	_, err = u.Exec(sql, utxo.Op.Bytes(), valueBytes, utxo.LockTime, utxo.AtHeight, utxo.Address.Bytes())
	return err
}

// get a utxo from database
func (u *utxos) Get(op *util.OutPoint) (*sutil.UTXO, error) {
	u.RLock()
	defer u.RUnlock()

	row := u.QueryRow(`SELECT Value, LockTime, AtHeight, Address FROM UTXOs WHERE OutPoint=?`, op.Bytes())
	var valueBytes []byte
	var lockTime uint32
	var atHeight uint32
	var addressBytes []byte
	err := row.Scan(&valueBytes, &lockTime, &atHeight, &addressBytes)
	if err != nil {
		return nil, err
	}

	value, err := common.Fixed64FromBytes(valueBytes)
	if err != nil {
		return nil, err
	}
	address, err := common.Uint168FromBytes(addressBytes)
	if err != nil {
		return nil, err
	}

	return &sutil.UTXO{Op: op, Value: *value, LockTime: lockTime, AtHeight: atHeight, Address: *address}, nil
}

// get utxos of the given script hash from database
func (u *utxos) GetAddrAll(hash *common.Uint168) ([]*sutil.UTXO, error) {
	u.RLock()
	defer u.RUnlock()

	rows, err := u.Query(
		"SELECT OutPoint, Value, LockTime, AtHeight, Address FROM UTXOs WHERE Address=?", hash.Bytes())
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return u.getUTXOs(rows)
}

func (u *utxos) GetAll() ([]*sutil.UTXO, error) {
	u.RLock()
	defer u.RUnlock()

	rows, err := u.Query("SELECT OutPoint, Value, LockTime, AtHeight, Address FROM UTXOs")
	if err != nil {
		return []*sutil.UTXO{}, err
	}
	defer rows.Close()

	return u.getUTXOs(rows)
}

func (u *utxos) getUTXOs(rows *sql.Rows) ([]*sutil.UTXO, error) {
	var utxos []*sutil.UTXO
	for rows.Next() {
		var opBytes []byte
		var valueBytes []byte
		var lockTime uint32
		var atHeight uint32
		var addressBytes []byte
		err := rows.Scan(&opBytes, &valueBytes, &lockTime, &atHeight, &addressBytes)
		if err != nil {
			return utxos, err
		}

		outPoint, err := util.OutPointFromBytes(opBytes)
		if err != nil {
			return utxos, err
		}
		value, err := common.Fixed64FromBytes(valueBytes)
		if err != nil {
			return utxos, err
		}
		address, err := common.Uint168FromBytes(addressBytes)
		if err != nil {
			return utxos, err
		}
		utxo := &sutil.UTXO{Op: outPoint, Value: *value, LockTime: lockTime,
			AtHeight: atHeight, Address: *address}
		utxos = append(utxos, utxo)
	}

	return utxos, nil
}

// delete a utxo from database
func (u *utxos) Del(op *util.OutPoint) error {
	u.Lock()
	defer u.Unlock()

	_, err := u.Exec("DELETE FROM UTXOs WHERE OutPoint=?", op.Bytes())
	return err
}

func (u *utxos) Batch() UTXOsBatch {
	u.Lock()
	defer u.Unlock()

	tx, err := u.DB.Begin()
	if err != nil {
		panic(err)
	}

	return &utxosBatch{
		RWMutex: u.RWMutex,
		Tx:      tx,
	}
}
