package store

import (
	"bytes"
	"database/sql"
	"encoding/binary"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/core"
	"github.com/syndtr/goleveldb/leveldb"
)

// Ensure dataBatch implement DataBatch interface.
var _ DataBatch = (*dataBatch)(nil)

type dataBatch struct {
	mutex sync.Mutex
	*leveldb.DB
	*leveldb.Batch
	sqlTx *sql.Tx
}

func (b *dataBatch) Txs() TxsBatch {
	return &txsBatch{Batch: b.Batch}
}

func (b *dataBatch) Ops() OpsBatch {
	return &opsBatch{Batch: b.Batch}
}

func (b *dataBatch) Que() QueBatch {
	return &queBatch{Tx: b.sqlTx}
}

// Delete all transactions, ops, queued items on the given height.
func (b *dataBatch) DelAll(height uint32) error {
	b.mutex.Lock()
	defer b.mutex.Unlock()

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], height)
	data, _ := b.DB.Get(toKey(BKTHeightTxs, key[:]...), nil)
	for _, txId := range getTxIds(data) {
		var utx util.Tx
		data, err := b.DB.Get(toKey(BKTTxs, txId.Bytes()...), nil)
		if err != nil {
			return err
		}
		if err := utx.Deserialize(bytes.NewReader(data)); err != nil {
			return err
		}

		var tx core.Transaction
		err = tx.Deserialize(bytes.NewReader(utx.RawData))
		if err != nil {
			return err
		}

		for index := range tx.Outputs {
			outpoint := core.NewOutPoint(utx.Hash, uint16(index))
			b.Batch.Delete(toKey(BKTOps, outpoint.Bytes()...))
		}

		b.Batch.Delete(toKey(BKTTxs, txId.Bytes()...))
	}

	b.Batch.Delete(toKey(BKTHeightTxs, key[:]...))

	return b.Que().DelAll(height)
}

func (b *dataBatch) Commit() error {
	defer b.sqlTx.Rollback()

	if err := b.DB.Write(b.Batch, nil); err != nil {
		return err
	}

	if err := b.sqlTx.Commit(); err != nil {
		return err
	}

	return nil
}

func (b *dataBatch) Rollback() error {
	b.Batch.Reset()
	return b.sqlTx.Rollback()
}
