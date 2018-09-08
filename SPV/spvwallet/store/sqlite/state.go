package db

import (
	"database/sql"
	"sync"
)

const CreateStateDB = `CREATE TABLE IF NOT EXISTS State(
				Key NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL
			);`

const (
	HeightKey = "Height"
)

type StateDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewStateDB(db *sql.DB, lock *sync.RWMutex) (*StateDB, error) {
	_, err := db.Exec(CreateStateDB)
	if err != nil {
		return nil, err
	}
	return &StateDB{RWMutex: lock, DB: db}, nil
}

// get state height
func (db *StateDB) GetHeight() uint32 {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow("SELECT Value FROM State WHERE Key=?", HeightKey)

	var height uint32
	err := row.Scan(&height)
	if err != nil {
		return 0
	}

	return height
}

// save state height
func (db *StateDB) PutHeight(height uint32) {
	db.Lock()
	defer db.Unlock()

	db.Exec("INSERT OR REPLACE INTO State(Key, Value) VALUES(?,?)", HeightKey, height)
}
