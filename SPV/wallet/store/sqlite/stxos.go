package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
)

const CreateSTXOsDB = `CREATE TABLE IF NOT EXISTS STXOs(
				OutPoint BLOB NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL,
				LockTime INTEGER NOT NULL,
				AtHeight INTEGER NOT NULL,
				SpendHash BLOB NOT NULL,
				SpendHeight INTEGER NOT NULL,
				Address BLOB NOT NULL
			);`

// Ensure stxos implement STXOs interface.
var _ STXOs = (*stxos)(nil)

type stxos struct {
	*sync.RWMutex
	*sql.DB
}

func NewSTXOs(db *sql.DB, lock *sync.RWMutex) (*stxos, error) {
	_, err := db.Exec(CreateSTXOsDB)
	if err != nil {
		return nil, err
	}
	return &stxos{RWMutex: lock, DB: db}, nil
}

// Put save a UTXO into database
func (s *stxos) Put(stxo *sutil.STXO) error {
	s.Lock()
	defer s.Unlock()

	valueBytes, err := stxo.Value.Bytes()
	if err != nil {
		return err
	}
	sql := `INSERT OR REPLACE INTO STXOs(OutPoint, Value, LockTime, AtHeight,
			SpendHash, SpendHeight, Address) VALUES(?,?,?,?,?,?,?)`
	_, err = s.Exec(sql, stxo.Op.Bytes(), valueBytes, stxo.LockTime, stxo.AtHeight,
		stxo.SpendTxId.Bytes(), stxo.SpendHeight, stxo.Address.Bytes())
	return err
}

// get a stxo from database
func (s *stxos) Get(op *util.OutPoint) (*sutil.STXO, error) {
	s.RLock()
	defer s.RUnlock()

	sql := `SELECT Value, LockTime, AtHeight, SpendHash, SpendHeight, Address FROM STXOs WHERE OutPoint=?`
	row := s.QueryRow(sql, op.Bytes())
	var valueBytes []byte
	var lockTime uint32
	var atHeight uint32
	var spendHashBytes []byte
	var spendHeight uint32
	var addressBytes []byte
	err := row.Scan(&valueBytes, &lockTime, &atHeight, &spendHashBytes, &spendHeight, &addressBytes)
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

	var utxo = sutil.UTXO{Op: op, Value: *value, LockTime: lockTime, AtHeight: atHeight, Address: *address}
	spendHash, err := common.Uint256FromBytes(spendHashBytes)
	if err != nil {
		return nil, err
	}

	return &sutil.STXO{UTXO: utxo, SpendTxId: *spendHash, SpendHeight: spendHeight}, nil
}

// get stxos of the given script hash from database
func (s *stxos) GetAddrAll(hash *common.Uint168) ([]*sutil.STXO, error) {
	s.RLock()
	defer s.RUnlock()

	sql := `SELECT OutPoint, Value, LockTime, AtHeight, SpendHash,
			SpendHeight, Address FROM STXOs WHERE Address=?`
	rows, err := s.Query(sql, hash.Bytes())
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return s.getSTXOs(rows)
}

func (s *stxos) GetAll() ([]*sutil.STXO, error) {
	s.RLock()
	defer s.RUnlock()

	sql := "SELECT OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight, Address FROM STXOs"
	rows, err := s.Query(sql)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	return s.getSTXOs(rows)
}

func (s *stxos) getSTXOs(rows *sql.Rows) ([]*sutil.STXO, error) {
	var stxos []*sutil.STXO
	for rows.Next() {
		var opBytes []byte
		var valueBytes []byte
		var lockTime uint32
		var atHeight uint32
		var spendHashBytes []byte
		var spendHeight uint32
		var addressBytes []byte
		err := rows.Scan(&opBytes, &valueBytes, &lockTime, &atHeight,
			&spendHashBytes, &spendHeight, &addressBytes)
		if err != nil {
			return stxos, err
		}

		outPoint, err := util.OutPointFromBytes(opBytes)
		if err != nil {
			return stxos, err
		}
		value, err := common.Fixed64FromBytes(valueBytes)
		if err != nil {
			return stxos, err
		}
		address, err := common.Uint168FromBytes(addressBytes)
		if err != nil {
			return stxos, err
		}
		var utxo = sutil.UTXO{Op: outPoint, Value: *value, LockTime: lockTime,
			AtHeight: atHeight, Address: *address}
		spendHash, err := common.Uint256FromBytes(spendHashBytes)
		if err != nil {
			return stxos, err
		}

		stxos = append(stxos, &sutil.STXO{UTXO: utxo, SpendTxId: *spendHash,
			SpendHeight: spendHeight})
	}

	return stxos, nil
}

// delete a stxo from database
func (s *stxos) Del(op *util.OutPoint) error {
	s.Lock()
	defer s.Unlock()

	_, err := s.Exec("DELETE FROM STXOs WHERE OutPoint=?", op.Bytes())
	return err
}

func (s *stxos) Batch() STXOsBatch {
	s.Lock()
	defer s.Unlock()

	tx, err := s.DB.Begin()
	if err != nil {
		panic(err)
	}

	return &stxosBatch{
		RWMutex: s.RWMutex,
		Tx:      tx,
	}
}
