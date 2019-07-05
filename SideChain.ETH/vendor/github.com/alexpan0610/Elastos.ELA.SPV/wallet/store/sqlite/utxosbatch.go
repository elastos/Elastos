package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"
)

// Ensure utxos implement UTXOs interface.
var _ UTXOsBatch = (*utxosBatch)(nil)

type utxosBatch struct {
	*sync.RWMutex
	*sql.Tx
}

// put a utxo to database
func (db *utxosBatch) Put(utxo *sutil.UTXO) error {
	db.Lock()
	defer db.Unlock()

	valueBytes, err := utxo.Value.Bytes()
	if err != nil {
		return err
	}
	sql := "INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, Address) VALUES(?,?,?,?,?)"
	_, err = db.Exec(sql, utxo.Op.Bytes(), valueBytes, utxo.LockTime, utxo.AtHeight, utxo.Address.Bytes())
	return err
}

// delete a utxo from database
func (db *utxosBatch) Del(op *util.OutPoint) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM UTXOs WHERE OutPoint=?", op.Bytes())
	return err
}
