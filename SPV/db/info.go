package db

import (
	"database/sql"
	"sync"
	"encoding/binary"
	"bytes"
)

const CreateInfoDB = `CREATE TABLE IF NOT EXISTS Info(
				Key NOT NULL PRIMARY KEY,
				Value BLOB NOT NULL
			);`

const (
	ChainHeightKey = "ChainHeight"
)

type InfoDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewInfoDB(db *sql.DB, lock *sync.RWMutex) (Info, error) {
	_, err := db.Exec(CreateInfoDB)
	if err != nil {
		return nil, err
	}
	return &InfoDB{RWMutex: lock, DB: db}, nil
}

// get chain height
func (db *InfoDB) ChainHeight() uint32 {
	value, err := db.Get(ChainHeightKey)
	if err != nil {
		return 0
	}

	var height uint32
	binary.Read(bytes.NewReader(value), binary.LittleEndian, &height)
	return height
}

// save chain height
func (db *InfoDB) SaveChainHeight(height uint32) {
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, height)
	db.Put(ChainHeightKey, buf.Bytes())
}

// put key and value into db
func (db *InfoDB) Put(key string, value []byte) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("INSERT OR REPLACE INTO Info(Key, Value) VALUES(?,?)", key, value)
	if err != nil {
		return err
	}

	return nil
}

// get value by key
func (db *InfoDB) Get(key string) ([]byte, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow("SELECT Value FROM Info WHERE Key=?", key)

	var value []byte
	err := row.Scan(&value)
	if err != nil {
		return nil, err
	}

	return value, nil
}

// delete value by key
func (db *InfoDB) Delete(key string) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Info WHERE key=?", key)
	if err != nil {
		return err
	}

	return nil
}
