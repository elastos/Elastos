package db

import (
	"sync"
	"database/sql"

	. "github.com/elastos/Elastos.ELA.SPV/common"
)

const CreateQueueDB = `CREATE TABLE IF NOT EXISTS Queue(
				TxHash BLOB NOT NULL PRIMARY KEY,
				BlockHash BLOB NOT NULL,
				Height INTEGER NOT NULL
			);`

type QueueDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewQueueDB(db *sql.DB, lock *sync.RWMutex) (Queue, error) {
	_, err := db.Exec(CreateQueueDB)
	if err != nil {
		return nil, err
	}
	return &QueueDB{RWMutex: lock, DB: db}, nil
}

// Put a queue item to database
func (db *QueueDB) Put(item *QueueItem) error {
	db.Lock()
	defer db.Unlock()

	sql := "INSERT OR REPLACE INTO Queue(TxHash, BlockHash, Height) VALUES(?,?,?)"
	stmt, err := db.Prepare(sql)
	if err != nil {
		return err
	}
	_, err = stmt.Exec(item.TxHash.Bytes(), item.BlockHash.Bytes(), item.Height)
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

		txHash, err := Uint256FromBytes(txHashBytes)
		if err != nil {
			return nil, err
		}
		blockHash, err := Uint256FromBytes(blockHashBytes)
		if err != nil {
			return nil, err
		}
		item := &QueueItem{TxHash: *txHash, BlockHash: *blockHash, Height: height}
		items = append(items, item)
	}

	return items, nil
}

// Delete confirmed item in queue
func (db *QueueDB) Delete(txHash *Uint256) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Queue WHERE TxHash=?", txHash.Bytes())
	if err != nil {
		return err
	}

	return nil
}
