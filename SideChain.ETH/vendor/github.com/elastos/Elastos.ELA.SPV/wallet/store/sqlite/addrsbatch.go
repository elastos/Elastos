package sqlite

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
)

// Ensure addrsBatch implement AddrsBatch interface.
var _ AddrsBatch = (*addrsBatch)(nil)

type addrsBatch struct {
	*sync.RWMutex
	*sql.Tx
}

// put a script to database
func (b *addrsBatch) Put(hash *common.Uint168, script []byte, addrType int) error {
	b.Lock()
	defer b.Unlock()

	sql := "INSERT OR REPLACE INTO Addrs(Hash, Script, Type) VALUES(?,?,?)"
	_, err := b.Exec(sql, hash.Bytes(), script, addrType)
	if err != nil {
		return err
	}

	return nil
}

// delete a script from database
func (b *addrsBatch) Del(hash *common.Uint168) error {
	b.Lock()
	defer b.Unlock()

	_, err := b.Exec("DELETE FROM Addrs WHERE Hash=?", hash.Bytes())
	if err != nil {
		return err
	}

	return nil
}
