package db

import (
	"database/sql"
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/sdk"
)

const CreateAddrsDB = `CREATE TABLE IF NOT EXISTS Addrs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB,
				Type INTEGER NOT NULL
			);`

type AddrsDB struct {
	*sync.RWMutex
	*sql.DB
	filter *sdk.AddrFilter
}

func NewAddrsDB(db *sql.DB, lock *sync.RWMutex) (Addrs, error) {
	_, err := db.Exec(CreateAddrsDB)
	if err != nil {
		return nil, err
	}
	return &AddrsDB{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (db *AddrsDB) Put(hash *Uint168, script []byte, addrType int) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("INSERT OR REPLACE INTO Addrs(Hash, Script, Type) VALUES(?,?,?)")
	if err != nil {
		return err
	}
	_, err = stmt.Exec(hash.ToArray(), script, addrType)
	if err != nil {
		return err
	}

	db.getFilter().AddAddr(NewAddr(hash, script, addrType).hash)

	return nil
}

// get a script from database
func (db *AddrsDB) Get(hash *Uint168) (*Addr, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Script, Type FROM Addrs WHERE Hash=?`, hash.ToArray())
	var script []byte
	var addrType int
	err := row.Scan(&script, &addrType)
	if err != nil {
		return nil, err
	}

	return NewAddr(hash, script, addrType), nil
}

// get all Addrs from database
func (db *AddrsDB) GetAll() ([]*Addr, error) {
	db.RLock()
	defer db.RUnlock()

	return db.getAll()
}

func (db *AddrsDB) getAll() ([]*Addr, error) {
	var addrs []*Addr

	rows, err := db.Query("SELECT Hash, Script, Type FROM Addrs")
	if err != nil {
		return addrs, err
	}
	defer rows.Close()

	for rows.Next() {
		var hashBytes []byte
		var script []byte
		var addrType int
		err = rows.Scan(&hashBytes, &script, &addrType)
		if err != nil {
			return addrs, err
		}
		hash, err := Uint168FromBytes(hashBytes)
		if err != nil {
			return addrs, err
		}
		addrs = append(addrs, NewAddr(hash, script, addrType))
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
func (db *AddrsDB) GetAddrFilter() *sdk.AddrFilter {
	db.RLock()
	defer db.RUnlock()

	return db.getFilter()
}

// reload filter from db
func (db *AddrsDB) ReloadAddrFilter() *sdk.AddrFilter {
	db.RLock()
	defer db.RUnlock()

	return db.loadFilter()
}

func (db *AddrsDB) getFilter() *sdk.AddrFilter {
	if db.filter == nil {
		db.loadFilter()
	}
	return db.filter
}

func (db *AddrsDB) loadFilter() *sdk.AddrFilter {
	addrs, _ := db.getAll()
	db.filter = sdk.NewAddrFilter(nil)
	for _, addr := range addrs {
		db.filter.AddAddr(addr.hash)
	}
	return db.filter
}
