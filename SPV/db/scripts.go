package db

import (
	"database/sql"
	"sync"

	. "SPVWallet/core"
)

const CreateScriptsDB = `CREATE TABLE IF NOT EXISTS Scripts(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB UNIQUE NOT NULL
			);`

type ScriptsDB struct {
	*sync.RWMutex
	*sql.DB
	filter *ScriptFilter
}

func NewScriptDB(db *sql.DB, lock *sync.RWMutex) (Scripts, error) {
	_, err := db.Exec(CreateScriptsDB)
	if err != nil {
		return nil, err
	}
	return &ScriptsDB{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (db *ScriptsDB) Put(hash *Uint168, script []byte) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("INSERT OR REPLACE INTO Scripts(Hash, Script) VALUES(?,?)")
	if err != nil {
		return err
	}
	_, err = stmt.Exec(hash.ToArray(), script)
	if err != nil {
		return err
	}

	db.getFilter().AddScript(script)

	return nil
}

// get a script from database
func (db *ScriptsDB) Get(hash *Uint168) ([]byte, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Script FROM Scripts WHERE Hash=?`, hash.ToArray())
	var script []byte
	err := row.Scan(&script)
	if err != nil {
		return nil, err
	}

	return script, nil
}

// get all scripts from database
func (db *ScriptsDB) GetAll() ([][]byte, error) {
	db.RLock()
	defer db.RUnlock()

	return db.getAll()
}

func (db *ScriptsDB) getAll() ([][]byte, error) {
	var scripts [][]byte

	rows, err := db.Query("SELECT Script FROM Scripts")
	if err != nil {
		return scripts, err
	}
	defer rows.Close()

	for rows.Next() {
		var script []byte
		err = rows.Scan(&script)
		if err != nil {
			return scripts, err
		}
		scripts = append(scripts, script)
	}

	return scripts, nil
}

// delete a script from database
func (db *ScriptsDB) Delete(hash *Uint168) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("DELETE FROM Scripts WHERE Hash=?")
	if err != nil {
		return err
	}
	_, err = stmt.Exec(hash.ToArray())
	if err != nil {
		return err
	}

	db.getFilter().DeleteScript(*hash)

	return nil
}

// get scripts filter
func (db *ScriptsDB) GetFilter() *ScriptFilter {
	db.RLock()
	defer db.RUnlock()

	return db.getFilter()
}

func (db *ScriptsDB) getFilter() *ScriptFilter {
	if db.filter == nil {
		scripts, _ := db.getAll()
		db.filter = NewScriptFilter(scripts)
	}
	return db.filter
}
