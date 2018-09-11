package sqlite

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

// Ensure state implement State interface.
var _ State = (*state)(nil)

type state struct {
	*sync.RWMutex
	*sql.DB
}

func NewState(db *sql.DB, lock *sync.RWMutex) (*state, error) {
	_, err := db.Exec(CreateStateDB)
	if err != nil {
		return nil, err
	}
	return &state{RWMutex: lock, DB: db}, nil
}

// get state height
func (s *state) GetHeight() uint32 {
	s.RLock()
	defer s.RUnlock()

	row := s.QueryRow("SELECT Value FROM State WHERE Key=?", HeightKey)

	var height uint32
	err := row.Scan(&height)
	if err != nil {
		return 0
	}

	return height
}

// save state height
func (s *state) PutHeight(height uint32) {
	s.Lock()
	defer s.Unlock()

	s.Exec("INSERT OR REPLACE INTO State(Key, Value) VALUES(?,?)", HeightKey, height)
}
