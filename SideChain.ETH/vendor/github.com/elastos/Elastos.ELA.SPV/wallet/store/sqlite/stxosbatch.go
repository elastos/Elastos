package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"
)

// Ensure stxos implement STXOs interface.
var _ STXOsBatch = (*stxosBatch)(nil)

type stxosBatch struct {
	*sync.RWMutex
	*sql.Tx
}

// Put save a UTXO into database
func (s *stxosBatch) Put(stxo *sutil.STXO) error {
	s.Lock()
	defer s.Unlock()

	valueBytes, err := stxo.Value.Bytes()
	if err != nil {
		return err
	}
	sql := `INSERT OR REPLACE INTO STXOs(OutPoint, Value, LockTime, AtHeight, SpendHash, SpendHeight, Address)
			VALUES(?,?,?,?,?,?,?)`
	_, err = s.Exec(sql, stxo.Op.Bytes(), valueBytes, stxo.LockTime, stxo.AtHeight,
		stxo.SpendTxId.Bytes(), stxo.SpendHeight, stxo.Address.Bytes())
	return err
}

// delete a stxo from database
func (sb *stxosBatch) Del(outPoint *util.OutPoint) error {
	sb.Lock()
	defer sb.Unlock()

	_, err := sb.Exec("DELETE FROM STXOs WHERE OutPoint=?", outPoint.Bytes())
	if err != nil {
		return err
	}

	return nil
}
