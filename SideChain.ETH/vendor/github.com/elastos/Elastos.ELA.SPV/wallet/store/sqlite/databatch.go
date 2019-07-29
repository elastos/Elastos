package sqlite

import (
	"database/sql"
	"sync"
)

// Ensure dataBatch implement DataStore interface
var _ DataBatch = (*dataBatch)(nil)

type dataBatch struct {
	*sync.RWMutex
	*sql.Tx
}

func (d *dataBatch) Addrs() AddrsBatch {
	d.Lock()
	defer d.Unlock()

	return &addrsBatch{
		RWMutex: d.RWMutex,
		Tx:      d.Tx,
	}
}

func (d *dataBatch) Txs() TxsBatch {
	d.Lock()
	defer d.Unlock()

	return &txsBatch{
		RWMutex: d.RWMutex,
		Tx:      d.Tx,
	}
}

func (d *dataBatch) UTXOs() UTXOsBatch {
	d.Lock()
	defer d.Unlock()

	return &utxosBatch{
		RWMutex: d.RWMutex,
		Tx:      d.Tx,
	}
}

func (d *dataBatch) STXOs() STXOsBatch {
	d.Lock()
	defer d.Unlock()

	return &stxosBatch{
		RWMutex: d.RWMutex,
		Tx:      d.Tx,
	}
}

func (d *dataBatch) RollbackHeight(height uint32) error {
	d.Lock()
	defer d.Unlock()

	// Rollback UTXOs
	_, err := d.Exec("DELETE FROM UTXOs WHERE AtHeight=?", height)
	if err != nil {
		return err
	}

	// Rollback STXOs, move UTXOs back first, then delete the STXOs
	_, err = d.Exec(`INSERT OR REPLACE INTO UTXOs(OutPoint, Value, LockTime, AtHeight, Address)
						SELECT OutPoint, Value, LockTime, AtHeight, Address FROM STXOs WHERE SpendHeight=?`, height)
	if err != nil {
		return err
	}
	_, err = d.Exec("DELETE FROM STXOs WHERE SpendHeight=?", height)
	if err != nil {
		return err
	}

	// Rollback TXNs
	_, err = d.Exec("DELETE FROM TXNs WHERE Height=?", height)
	return err
}
