package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
)

// Ensure addrs implement Addrs interface.
var _ Addrs = (*addrs)(nil)

const CreateAddrsDB = `CREATE TABLE IF NOT EXISTS Addrs(
				Hash BLOB NOT NULL PRIMARY KEY,
				Script BLOB,
				Type INTEGER NOT NULL
			);`

type addrs struct {
	*sync.RWMutex
	*sql.DB
}

func NewAddrs(db *sql.DB, lock *sync.RWMutex) (*addrs, error) {
	_, err := db.Exec(CreateAddrsDB)
	if err != nil {
		return nil, err
	}
	return &addrs{RWMutex: lock, DB: db}, nil
}

// put a script to database
func (a *addrs) Put(hash *common.Uint168, script []byte, addrType int) error {
	a.Lock()
	defer a.Unlock()

	sql := "INSERT OR REPLACE INTO Addrs(Hash, Script, Type) VALUES(?,?,?)"
	_, err := a.Exec(sql, hash.Bytes(), script, addrType)
	return err
}

// get a script from database
func (a *addrs) Get(hash *common.Uint168) (*sutil.Addr, error) {
	a.RLock()
	defer a.RUnlock()

	row := a.QueryRow(`SELECT Script, Type FROM Addrs WHERE Hash=?`, hash.Bytes())
	var script []byte
	var addrType int
	err := row.Scan(&script, &addrType)
	if err != nil {
		return nil, err
	}

	return sutil.NewAddr(hash, script, addrType), nil
}

// get all Addrs from database
func (a *addrs) GetAll() ([]*sutil.Addr, error) {
	a.RLock()
	defer a.RUnlock()

	var addrs []*sutil.Addr
	rows, err := a.Query("SELECT Hash, Script, Type FROM Addrs")
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
		addrs = append(addrs, sutil.NewAddr(hash, script, addrType))
	}

	return addrs, nil
}

// delete a script from database
func (a *addrs) Del(hash *common.Uint168) error {
	a.Lock()
	defer a.Unlock()

	_, err := a.Exec("DELETE FROM Addrs WHERE Hash=?", hash.Bytes())
	return err
}

func (a *addrs) Batch() AddrsBatch {
	a.Lock()
	defer a.Unlock()

	tx, err := a.DB.Begin()
	if err != nil {
		panic(err)
	}

	return &addrsBatch{
		RWMutex: a.RWMutex,
		Tx:      tx,
	}
}
