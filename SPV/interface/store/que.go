package store

import (
	"database/sql"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	DriverName = "sqlite3"
	DBName     = "./queue.db"

	CreateQueueDB = `CREATE TABLE IF NOT EXISTS Queue(
				NotifyId BLOB NOT NULL,
				TxId BLOB NOT NULL,
				Height INTEGER NOT NULL);
				CREATE INDEX IF NOT EXISTS idx_queue_notify_id ON Queue (NotifyId);
				CREATE INDEX IF NOT EXISTS idx_queue_tx_id ON Queue (TxId);
				CREATE INDEX IF NOT EXISTS idx_queue_height ON Queue (height);`
)

// Ensure que implement Que interface.
var _ Que = (*que)(nil)

type que struct {
	*sync.RWMutex
	*sql.DB
}

func NewQue() (*que, error) {
	db, err := sql.Open(DriverName, DBName)
	if err != nil {
		fmt.Println("Open sqlite db error:", err)
		return nil, err
	}

	_, err = db.Exec(CreateQueueDB)
	if err != nil {
		return nil, err
	}
	return &que{RWMutex: new(sync.RWMutex), DB: db}, nil
}

// Put a queue item to database
func (q *que) Put(item *QueItem) error {
	q.Lock()
	defer q.Unlock()

	sql := "INSERT OR REPLACE INTO Queue(NotifyId, TxId, Height) VALUES(?,?,?)"
	_, err := q.Exec(sql, item.NotifyId.Bytes(), item.TxId.Bytes(), item.Height)
	return err
}

// Get all items in queue
func (q *que) GetAll() ([]*QueItem, error) {
	q.RLock()
	defer q.RUnlock()

	rows, err := q.Query("SELECT NotifyId, TxId, Height FROM Queue")
	if err != nil {
		return nil, err
	}

	var items []*QueItem
	for rows.Next() {
		var notifyIdBytes []byte
		var txHashBytes []byte
		var height uint32
		err = rows.Scan(&notifyIdBytes, &txHashBytes, &height)
		if err != nil {
			return nil, err
		}

		notifyId, err := common.Uint256FromBytes(notifyIdBytes)
		if err != nil {
			return nil, err
		}
		txHash, err := common.Uint256FromBytes(txHashBytes)
		if err != nil {
			return nil, err
		}
		item := &QueItem{NotifyId: *notifyId, TxId: *txHash, Height: height}
		items = append(items, item)
	}

	return items, nil
}

// Delete confirmed item in queue
func (q *que) Del(notifyId, txHash *common.Uint256) error {
	q.Lock()
	defer q.Unlock()

	_, err := q.Exec("DELETE FROM Queue WHERE NotifyId=? AND TxId=?", notifyId.Bytes(), txHash.Bytes())
	return err
}

func (q *que) Batch() QueBatch {
	tx, err := q.Begin()
	if err != nil {
		panic(err)
	}
	return &queBatch{Tx: tx}
}

func (q *que) Clear() error {
	q.Lock()
	defer q.Unlock()

	_, err := q.Exec("DROP TABLE if EXISTS Queue")
	return err
}

func (q *que) Close() error {
	q.Lock()
	defer q.Unlock()
	return q.DB.Close()
}
