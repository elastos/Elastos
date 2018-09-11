package sqlite

import (
	"bytes"
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/sutil"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure txs implement Txs interface.
var _ TxsBatch = (*txsBatch)(nil)

type txsBatch struct {
	*sync.RWMutex
	*sql.Tx
}

// Put a new transaction to database
func (t *txsBatch) Put(storeTx *sutil.Tx) error {
	t.Lock()
	defer t.Unlock()

	buf := new(bytes.Buffer)
	err := storeTx.Data.SerializeUnsigned(buf)
	if err != nil {
		return err
	}

	sql := `INSERT OR REPLACE INTO Txs(Hash, Height, Timestamp, RawData) VALUES(?,?,?,?)`
	_, err = t.Exec(sql, storeTx.TxId.Bytes(), storeTx.Height, storeTx.Timestamp.Unix(), buf.Bytes())
	return err
}

// Delete a transaction from the db
func (t *txsBatch) Del(txId *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	_, err := t.Exec("DELETE FROM Txs WHERE Hash=?", txId.Bytes())
	return err
}
