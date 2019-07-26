package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

// Ensure txs implement Txs interface.
var _ TxsBatch = (*txsBatch)(nil)

type txsBatch struct {
	*sync.RWMutex
	*sql.Tx
}

// Put a new transaction to database
func (t *txsBatch) Put(tx *util.Tx) error {
	t.Lock()
	defer t.Unlock()

	sql := `INSERT OR REPLACE INTO Txs(Hash, Height, Timestamp, RawData) VALUES(?,?,?,?)`
	_, err := t.Exec(sql, tx.Hash.Bytes(), tx.Height, tx.Timestamp.Unix(), tx.RawData)
	return err
}

// Delete a transaction from the db
func (t *txsBatch) Del(txId *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	_, err := t.Exec("DELETE FROM Txs WHERE Hash=?", txId.Bytes())
	return err
}
