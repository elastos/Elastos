package store

import (
	"bytes"
	"encoding/binary"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
)

// Ensure txsBatch implement TxsBatch interface.
var _ TxsBatch = (*txsBatch)(nil)

type txsBatch struct {
	sync.Mutex
	*leveldb.DB
	*leveldb.Batch
	addTxs []*util.Tx
	delTxs []*util.Tx
}

func (b *txsBatch) Put(tx *util.Tx) error {
	b.Lock()
	defer b.Unlock()

	buf := new(bytes.Buffer)
	if err := tx.Serialize(buf); err != nil {
		return err
	}

	b.Batch.Put(toKey(BKTTxs, tx.Hash.Bytes()...), buf.Bytes())
	b.addTxs = append(b.addTxs, tx)
	return nil
}

func (b *txsBatch) Del(txId *common.Uint256) error {
	b.Lock()
	defer b.Unlock()

	var tx util.Tx
	data, err := b.DB.Get(toKey(BKTTxs, txId.Bytes()...), nil)
	if err != nil {
		return err
	}
	if err := tx.Deserialize(bytes.NewReader(data)); err != nil {
		return err
	}

	b.Batch.Delete(toKey(BKTTxs, txId.Bytes()...))
	b.delTxs = append(b.delTxs, &tx)
	return nil
}

func (b *txsBatch) DelAll(height uint32) error {
	b.Lock()
	defer b.Unlock()

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], height)
	data, _ := b.DB.Get(toKey(BKTHeightTxs, key[:]...), nil)
	for _, txID := range getTxIds(data) {
		b.Batch.Delete(toKey(BKTTxs, txID.Bytes()...))
	}
	b.Batch.Delete(toKey(BKTHeightTxs, key[:]...))

	return nil
}

func (b *txsBatch) Rollback() error {
	b.Lock()
	defer b.Unlock()
	b.Batch.Reset()
	return nil
}

func (b *txsBatch) Commit() error {
	b.Lock()
	defer b.Unlock()

	// Put height index for added transactions.
	if len(b.addTxs) > 0 {
		groups := groupByHeight(b.addTxs)
		for height, txs := range groups {
			var key [4]byte
			binary.BigEndian.PutUint32(key[:], height)
			data, _ := b.DB.Get(toKey(BKTHeightTxs, key[:]...), nil)
			for _, tx := range txs {
				data = putTxId(data, &tx.Hash)
			}
			b.Batch.Put(toKey(BKTHeightTxs, key[:]...), data)
		}
	}

	// Update height index for deleted transactions.
	if len(b.delTxs) > 0 {
		groups := groupByHeight(b.delTxs)
		for height, txs := range groups {
			var key [4]byte
			binary.BigEndian.PutUint32(key[:], height)
			data, _ := b.DB.Get(toKey(BKTHeightTxs, key[:]...), nil)
			for _, tx := range txs {
				data = delTxId(data, &tx.Hash)
			}
			b.Batch.Delete(toKey(BKTHeightTxs, key[:]...))
		}
	}

	return b.DB.Write(b.Batch, nil)
}

func groupByHeight(txs []*util.Tx) map[uint32][]*util.Tx {
	txGroups := make(map[uint32][]*util.Tx)
	for _, tx := range txs {
		group, ok := txGroups[tx.Height]
		if !ok {
			txGroups[tx.Height] = append(group, tx)
		} else {
			group = append(group, tx)
		}
	}
	return txGroups
}
