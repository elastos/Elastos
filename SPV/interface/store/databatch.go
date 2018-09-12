package store

import (
	"bytes"
	"database/sql"
	"encoding/binary"
	"encoding/gob"
	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
	"sync"
)

// Ensure dataBatch implement DataBatch interface.
var _ DataBatch = (*dataBatch)(nil)

type dataBatch struct {
	mutex  sync.Mutex
	boltTx *bolt.Tx
	sqlTx  *sql.Tx
}

func (b *dataBatch) Txs() TxsBatch {
	return &txsBatch{Tx: b.boltTx}
}

func (b *dataBatch) Ops() OpsBatch {
	return &opsBatch{Tx: b.boltTx}
}

func (b *dataBatch) Que() QueBatch {
	return &queBatch{Tx: b.sqlTx}
}

// Delete all transactions, ops, queued items on
// the given height.
func (b *dataBatch) DelAll(height uint32) error {
	b.mutex.Lock()
	defer b.mutex.Unlock()

	var key [4]byte
	binary.LittleEndian.PutUint32(key[:], height)
	data := b.boltTx.Bucket(BKTHeightTxs).Get(key[:])

	var txMap = make(map[common.Uint256]uint32)
	err := gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
	if err != nil {
		return err
	}

	txsBucket := b.boltTx.Bucket(BKTTxs)
	opsBucket := b.boltTx.Bucket(BKTOps)
	for txId := range txMap {
		var txn util.Tx
		data := txsBucket.Get(txId.Bytes())
		err := txn.Deserialize(bytes.NewReader(data))
		if err != nil {
			return err
		}

		for index := range txn.Outputs {
			outpoint := core.NewOutPoint(txn.Hash(), uint16(index)).Bytes()
			opsBucket.Delete(outpoint)
		}

		if err := b.boltTx.Bucket(BKTTxs).Delete(txId.Bytes()); err != nil {
			return err
		}
	}

	err =  b.boltTx.Bucket(BKTHeightTxs).Delete(key[:])
	if err != nil {
		return err
	}

	return b.Que().DelAll(height)
}

func (b *dataBatch) Commit() error {
	if err := b.boltTx.Commit(); err != nil {
		return err
	}

	if err := b.sqlTx.Commit(); err != nil {
		return err
	}

	return nil
}

func (b *dataBatch) Rollback() error {
	if err := b.boltTx.Rollback(); err != nil {
		return err
	}

	if err := b.sqlTx.Rollback(); err != nil {
		return err
	}

	return nil
}
