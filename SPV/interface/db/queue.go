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
	Delete(txHash *common.Uint256) error

	// Rollback queue items
	Rollback(height uint32) error

	// Reset clear all data in database
	Reset() error
}

const (
	DriverName = "sqlite3"
	DBName     = "./queue.db"

	CreateQueueDB = `CREATE TABLE IF NOT EXISTS Queue(
				TxHash BLOB NOT NULL PRIMARY KEY,
				BlockHash BLOB NOT NULL,
				Height INTEGER NOT NULL
			);`
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

	sql := "INSERT OR REPLACE INTO Queue(TxHash, BlockHash, Height) VALUES(?,?,?)"
	_, err := db.Exec(sql, item.TxHash.Bytes(), item.BlockHash.Bytes(), item.Height)
	if err != nil {
		return err
	}

	return nil
}

// Get all items in queue
func (db *QueueDB) GetAll() ([]*QueueItem, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT TxHash, BlockHash, Height FROM Queue")
	if err != nil {
		return nil, err
	}

	var items []*QueueItem
	for rows.Next() {
		var txHashBytes []byte
		var blockHashBytes []byte
		var height uint32
		err = rows.Scan(&txHashBytes, &blockHashBytes, &height)
		if err != nil {
			return nil, err
		}

		txHash, err := common.Uint256FromBytes(txHashBytes)
		if err != nil {
			return nil, err
		}
		blockHash, err := common.Uint256FromBytes(blockHashBytes)
		if err != nil {
			return nil, err
		}
		item := &QueueItem{TxHash: *txHash, BlockHash: *blockHash, Height: height}
		items = append(items, item)
	}

	return items, nil
}

// Delete confirmed item in queue
func (db *QueueDB) Delete(txHash *common.Uint256) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Queue WHERE TxHash=?", txHash.Bytes())
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
