package db

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const CreateAddrsDB = `CREATE TABLE IF NOT EXISTS Addrs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB,
				Type INTEGER NOT NULL
			);`

type AddrsDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewAddrsDB(db *sql.DB, lock *sync.RWMutex) (Addrs, error) {
	_, err := db.Exec(CreateAddrsDB)
	if err != nil {
		return nil, err
	}
	return &AddrsDB{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (db *AddrsDB) Put(hash *common.Uint168, script []byte, addrType int) error {
	db.Lock()
	defer db.Unlock()

	sql := "INSERT OR REPLACE INTO Addrs(Hash, Script, Type) VALUES(?,?,?)"
	_, err := db.Exec(sql, hash.Bytes(), script, addrType)
	if err != nil {
		return err
	}

	return nil
}

// get a script from database
func (db *AddrsDB) Get(hash *common.Uint168) (*Addr, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Script, Type FROM Addrs WHERE Hash=?`, hash.Bytes())
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
		hash, err := common.Uint168FromBytes(hashBytes)
		if err != nil {
			return addrs, err
		}
		addrs = append(addrs, NewAddr(hash, script, addrType))
	}

	return addrs, nil
}

// delete a script from database
func (db *AddrsDB) Delete(hash *common.Uint168) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Addrs WHERE Hash=?", hash.Bytes())
	if err != nil {
		return err
	}

	return nil
}
