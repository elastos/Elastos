package sqlite

import (
	"bytes"
	"database/sql"
	"math"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

const CreateTxsDB = `CREATE TABLE IF NOT EXISTS Txs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Height INTEGER NOT NULL,
				Timestamp INTEGER NOT NULL,
				RawData BLOB NOT NULL
			);`

type TXs struct {
	*sync.RWMutex
	*sql.DB
}

func NewTxs(db *sql.DB, lock *sync.RWMutex) (*TXs, error) {
	_, err := db.Exec(CreateTxsDB)
	if err != nil {
		return nil, err
	}
	return &TXs{RWMutex: lock, DB: db}, nil
}

// Put a new transaction to database
func (t *TXs) Put(storeTx *util.Tx) error {
	t.Lock()
	defer t.Unlock()

	buf := new(bytes.Buffer)
	err := storeTx.Data.SerializeUnsigned(buf)
	if err != nil {
		return err
	}

	sql := `INSERT OR REPLACE INTO Txs(Hash, Height, Timestamp, RawData) VALUES(?,?,?,?)`
	_, err = t.Exec(sql, storeTx.TxId.Bytes(), storeTx.Height, storeTx.Timestamp.Unix(), buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

// Fetch a raw tx and it's metadata given a hash
func (t *TXs) Get(txId *common.Uint256) (*util.Tx, error) {
	t.RLock()
	defer t.RUnlock()

	row := t.QueryRow(`SELECT Height, Timestamp, RawData FROM Txs WHERE Hash=?`, txId.Bytes())
	var height uint32
	var timestamp int64
	var rawData []byte
	err := row.Scan(&height, &timestamp, &rawData)
	if err != nil {
		return nil, err
	}
	var tx core.Transaction
	err = tx.DeserializeUnsigned(bytes.NewReader(rawData))
	if err != nil {
		return nil, err
	}

	return &util.Tx{TxId: *txId, Height: height, Timestamp: time.Unix(timestamp, 0), Data: tx}, nil
}

// Fetch all transactions from database
func (t *TXs) GetAll() ([]*util.Tx, error) {
	return t.GetAllFrom(math.MaxUint32)
}

// Fetch all transactions from the given height
func (t *TXs) GetAllFrom(height uint32) ([]*util.Tx, error) {
	t.RLock()
	defer t.RUnlock()

	sql := "SELECT Hash, Height, Timestamp, RawData FROM Txs"
	if height != math.MaxUint32 {
		sql += " WHERE Height=?"
	}
	var txns []*util.Tx
	rows, err := t.Query(sql, height)
	if err != nil {
		return txns, err
	}
	defer rows.Close()

	for rows.Next() {
		var txIdBytes []byte
		var height uint32
		var timestamp int64
		var rawData []byte
		err := rows.Scan(&txIdBytes, &height, &timestamp, &rawData)
		if err != nil {
			return txns, err
		}

		txId, err := common.Uint256FromBytes(txIdBytes)
		if err != nil {
			return txns, err
		}

		var tx core.Transaction
		err = tx.DeserializeUnsigned(bytes.NewReader(rawData))
		if err != nil {
			return nil, err
		}

		txns = append(txns, &util.Tx{TxId: *txId, Height: height, Timestamp: time.Unix(timestamp, 0), Data: tx})
	}

	return txns, nil
}

// Delete a transaction from the db
func (t *TXs) Delete(txId *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	_, err := t.Exec("DELETE FROM Txs WHERE Hash=?", txId.Bytes())
	if err != nil {
		return err
	}

	return nil
}
