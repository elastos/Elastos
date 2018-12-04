package db

import (
	"database/sql"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

type Queue interface {
	// Put a queue item to database
	Put(item *QueueItem) error

	// Get all items in queue
	GetAll() ([]*QueueItem, error)

	// Delete confirmed item in queue
	Delete(notifyId, txHash *common.Uint256) error

	// Rollback queue items
	Rollback(height uint32) error

	// Reset clear all data in database
	Reset() error
}

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

type QueueDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewQueueDB() (Queue, error) {
	db, err := sql.Open(DriverName, DBName)
	if err != nil {
		fmt.Println("Open sqlite db error:", err)
		return nil, err
	}

	_, err = db.Exec(CreateQueueDB)
	if err != nil {
		return nil, err
	}
	return &QueueDB{RWMutex: new(sync.RWMutex), DB: db}, nil
}

// Put a queue item to database
func (db *QueueDB) Put(item *QueueItem) error {
	db.Lock()
	defer db.Unlock()

	sql := "INSERT OR REPLACE INTO Queue(NotifyId, TxId, Height) VALUES(?,?,?)"
	_, err := db.Exec(sql, item.NotifyId.Bytes(), item.TxId.Bytes(), item.Height)
	if err != nil {
		return err
	}

	return nil
}

// Get all items in queue
func (db *QueueDB) GetAll() ([]*QueueItem, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT NotifyId, TxId, Height FROM Queue")
	if err != nil {
		return nil, err
	}

	var items []*QueueItem
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
		item := &QueueItem{NotifyId: *notifyId, TxId: *txHash, Height: height}
		items = append(items, item)
	}

	return items, nil
}

// Delete confirmed item in queue
func (db *QueueDB) Delete(notifyId, txHash *common.Uint256) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Queue WHERE NotifyId=? AND TxId=?", notifyId.Bytes(), txHash.Bytes())
	if err != nil {
		return err
	}

	return nil
}

// Rollback queue items
func (db *QueueDB) Rollback(height uint32) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Queue WHERE Height=?", height)
	return err
}

func (db *QueueDB) Reset() error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DROP TABLE if EXISTS Queue")
	return err
}
