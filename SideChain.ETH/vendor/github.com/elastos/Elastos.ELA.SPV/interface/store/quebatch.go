package store

import (
	"database/sql"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure queBatch implement QueBatch interface.
var _ QueBatch = (*queBatch)(nil)

type queBatch struct {
	sync.Mutex
	*sql.Tx
}

// Put a queue item to database
func (b *queBatch) Put(item *QueItem) error {
	b.Lock()
	defer b.Unlock()

	sql := "INSERT OR REPLACE INTO Queue(NotifyId, TxId, Height) VALUES(?,?,?)"
	_, err := b.Tx.Exec(sql, item.NotifyId.Bytes(), item.TxId.Bytes(), item.Height)
	return err
}

// Delete confirmed item in queue
func (b *queBatch) Del(notifyId, txHash *common.Uint256) error {
	b.Lock()
	defer b.Unlock()

	_, err := b.Tx.Exec("DELETE FROM Queue WHERE NotifyId=? AND TxId=?", notifyId.Bytes(), txHash.Bytes())
	return err
}

// Delete all items on the given height.
func (b *queBatch) DelAll(height uint32) error {
	b.Lock()
	defer b.Unlock()

	_, err := b.Tx.Exec("DELETE FROM Queue WHERE Height=?", height)
	return err
}
