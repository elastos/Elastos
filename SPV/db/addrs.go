package db

import (
	"database/sql"
	"sync"

	. "SPVWallet/core"
)

const CreateAddrsDB = `CREATE TABLE IF NOT EXISTS Addrs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB
			);`

type AddrsDB struct {
	*sync.RWMutex
	*sql.DB
	filter *AddrFilter
}

func NewAddrsDB(db *sql.DB, lock *sync.RWMutex) (Addrs, error) {
	_, err := db.Exec(CreateAddrsDB)
	if err != nil {
		return nil, err
	}
	return &AddrsDB{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (db *AddrsDB) Put(hash *Uint168, script []byte) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("INSERT OR REPLACE INTO Addrs(Hash, Script) VALUES(?,?)")
	if err != nil {
		return err
	}
	_, err = stmt.Exec(hash.ToArray(), script)
	if err != nil {
		return err
	}

	db.getFilter().AddAddr(NewAddr(hash, script))

	return nil
}

// get a script from database
func (db *AddrsDB) Get(hash *Uint168) (*Addr, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Script FROM Addrs WHERE Hash=?`, hash.ToArray())
	var script []byte
	err := row.Scan(&script)
	if err != nil {
		return nil, err
	}

	return &Addr{hash: hash, script: script}, nil
}

// get all Addrs from database
func (db *AddrsDB) GetAll() ([]*Addr, error) {
	db.RLock()
	defer db.RUnlock()

	return db.getAll()
}

func (db *AddrsDB) getAll() ([]*Addr, error) {
	var addrs []*Addr

	rows, err := db.Query("SELECT Hash, Script FROM Addrs")
	if err != nil {
		return addrs, err
	}
	defer rows.Close()

	for rows.Next() {
		var hashBytes []byte
		var script []byte
		err = rows.Scan(&hashBytes, &script)
		if err != nil {
			return addrs, err
		}
		hash, err := Uint168FromBytes(hashBytes)
		if err != nil {
			return addrs, err
		}
		addrs = append(addrs, NewAddr(hash, script))
	}

	return addrs, nil
}

// delete a script from database
func (db *AddrsDB) Delete(hash *Uint168) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("DELETE FROM Addrs WHERE Hash=?")
	if err != nil {
		return err
	}
	_, err = stmt.Exec(hash.ToArray())
	if err != nil {
		return err
	}

	db.getFilter().DeleteAddr(*hash)

	return nil
}

// get Addrs filter
func (db *AddrsDB) GetAddrFilter() *AddrFilter {
	db.RLock()
	defer db.RUnlock()

	return db.getFilter()
}

// reload filter from db
func (db *AddrsDB) ReloadAddrFilter() *AddrFilter {
	db.RLock()
	defer db.RUnlock()

	return db.loadFilter()
}

func (db *AddrsDB) getFilter() *AddrFilter {
	if db.filter == nil {
		db.loadFilter()
	}
	return db.filter
}

func (db *AddrsDB) loadFilter() *AddrFilter {
	Addrs, _ := db.getAll()
	db.filter = NewAddrFilter(Addrs)
	return db.filter
}
