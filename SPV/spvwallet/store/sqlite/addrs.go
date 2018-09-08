package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/spvwallet/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const CreateAddrsDB = `CREATE TABLE IF NOT EXISTS Addrs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB,
				Type INTEGER NOT NULL
			);`

type Addrs struct {
	*sync.RWMutex
	*sql.DB
}

func NewAddrs(db *sql.DB, lock *sync.RWMutex) (*Addrs, error) {
	_, err := db.Exec(CreateAddrsDB)
	if err != nil {
		return nil, err
	}
	return &Addrs{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (db *Addrs) Put(hash *common.Uint168, script []byte, addrType int) error {
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
func (db *Addrs) Get(hash *common.Uint168) (*util.Addr, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow(`SELECT Script, Type FROM Addrs WHERE Hash=?`, hash.Bytes())
	var script []byte
	var addrType int
	err := row.Scan(&script, &addrType)
	if err != nil {
		return nil, err
	}

	return util.NewAddr(hash, script, addrType), nil
}

// get all Addrs from database
func (db *Addrs) GetAll() ([]*util.Addr, error) {
	db.RLock()
	defer db.RUnlock()

	var addrs []*util.Addr
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
		addrs = append(addrs, util.NewAddr(hash, script, addrType))
	}

	return addrs, nil
}

// delete a script from database
func (db *Addrs) Delete(hash *common.Uint168) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Addrs WHERE Hash=?", hash.Bytes())
	if err != nil {
		return err
	}

	return nil
}
