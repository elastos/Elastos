package db

import (
	"database/sql"
	"sync"
)

const CreateInfoDB = `CREATE TABLE IF NOT EXISTS Chain(
				Key NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL
			);`

const (
	HeightKey = "Height"
)

type ChainDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewChainDB(db *sql.DB, lock *sync.RWMutex) (Chain, error) {
	_, err := db.Exec(CreateInfoDB)
	if err != nil {
		return nil, err
	}
	return &ChainDB{RWMutex: lock, DB: db}, nil
}

// get chain height
func (db *ChainDB) GetHeight() uint32 {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow("SELECT Value FROM Chain WHERE Key=?", HeightKey)

	var height uint32
	err := row.Scan(&height)
	if err != nil {
		return 0
	}

	return height
}

// save chain height
func (db *ChainDB) PutHeight(height uint32) {
	db.Lock()
	defer db.Unlock()

	db.Exec("INSERT OR REPLACE INTO Chain(Key, Value) VALUES(?,?)", HeightKey, height)
}
