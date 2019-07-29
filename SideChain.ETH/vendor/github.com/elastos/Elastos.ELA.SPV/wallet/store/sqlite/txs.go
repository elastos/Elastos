package sqlite

import (
	"bytes"
	"database/sql"
	"math"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
)

const CreateTxsDB = `CREATE TABLE IF NOT EXISTS Txs(
					Hash BLOB NOT NULL PRIMARY KEY,
					Height INTEGER NOT NULL,
					Timestamp INTEGER NOT NULL,
					RawData BLOB NOT NULL
				);
				CREATE TABLE IF NOT EXISTS ForkTxs(
					Hash BLOB NOT NULL PRIMARY KEY,
					Num INTEGER NOT NULL,
					RawData BLOB NOT NULL
				);`

// Ensure txs implement Txs interface.
var _ Txs = (*txs)(nil)

type txs struct {
	*sync.RWMutex
	*sql.DB
}

func NewTxs(db *sql.DB, lock *sync.RWMutex) (*txs, error) {
	_, err := db.Exec(CreateTxsDB)
	if err != nil {
		return nil, err
	}
	return &txs{RWMutex: lock, DB: db}, nil
}

// Put a new transaction to database
func (t *txs) Put(tx *util.Tx) error {
	t.Lock()
	defer t.Unlock()

	sql := `INSERT OR REPLACE INTO Txs(Hash, Height, Timestamp, RawData) VALUES(?,?,?,?)`
	_, err := t.Exec(sql, tx.Hash.Bytes(), tx.Height, tx.Timestamp.Unix(), tx.RawData)
	return err
}

// Fetch a raw tx and it's metadata given a hash
func (t *txs) Get(txId *common.Uint256) (*util.Tx, error) {
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

	return &util.Tx{Hash: *txId, Height: height,
		Timestamp: time.Unix(timestamp, 0), RawData: rawData}, nil
}

// Fetch all transactions from database
func (t *txs) GetAll() ([]*util.Tx, error) {
	return t.GetAllFrom(math.MaxUint32)
}

// Fetch all unconfirmed transactions from database
func (t *txs) GetAllUnconfirmed() ([]*util.Tx, error) {
	return t.GetAllFrom(0)
}

// Fetch all transactions from the given height
func (t *txs) GetAllFrom(height uint32) ([]*util.Tx, error) {
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

		txns = append(txns, &util.Tx{Hash: *txId, Height: height,
			Timestamp: time.Unix(timestamp, 0), RawData: rawData})
	}

	return txns, nil
}

// Delete a transaction from the db
func (t *txs) Del(txId *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	_, err := t.Exec("DELETE FROM Txs WHERE Hash=?", txId.Bytes())
	return err
}

// PutForkTxs persists the fork chain transactions into database with the
// fork block hash and can be queried by GetForkTxs(hash).
func (t *txs) PutForkTxs(txs []*util.Tx, hash *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	buf := new(bytes.Buffer)
	for _, tx := range txs {
		if err := tx.Serialize(buf); err != nil {
			return err
		}
	}

	_, err := t.Exec(`INSERT OR REPLACE INTO ForkTxs(Hash, Num, RawData) VALUES(?,?,?)`,
		hash.Bytes(), len(txs), buf.Bytes())
	return err
}

// GetForkTxs returns all transactions within the fork block hash.
func (t *txs) GetForkTxs(hash *common.Uint256) ([]*util.Tx, error) {
	t.RLock()
	defer t.RUnlock()

	row := t.QueryRow(`SELECT Num, RawData FROM ForkTxs WHERE Hash=?`,
		hash.Bytes())
	var num int
	var data []byte
	if err := row.Scan(&num, &data); err != nil {
		return nil, err
	}

	txs := make([]*util.Tx, num)
	buf := bytes.NewReader(data)
	for i := range txs {
		var utx util.Tx
		if err := utx.Deserialize(buf); err != nil {
			return nil, err
		}
		txs[i] = &utx
	}
	return txs, nil
}

func (t *txs) Batch() TxsBatch {
	t.Lock()
	defer t.Unlock()

	tx, err := t.DB.Begin()
	if err != nil {
		panic(err)
	}

	return &txsBatch{
		RWMutex: t.RWMutex,
		Tx:      tx,
	}
}
