package store

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"github.com/boltdb/bolt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure txsBatch implement TxsBatch interface.
var _ TxsBatch = (*txsBatch)(nil)

type txsBatch struct {
	sync.Mutex
	*bolt.Tx
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

	err := b.Tx.Bucket(BKTTxs).Put(tx.Hash.Bytes(), buf.Bytes())
	if err != nil {
		return err
	}

	b.addTxs = append(b.addTxs, tx)
	return nil
}

func (b *txsBatch) Del(txId *common.Uint256) error {
	b.Lock()
	defer b.Unlock()

	var tx util.Tx
	data := b.Tx.Bucket(BKTTxs).Get(txId.Bytes())
	err := tx.Deserialize(bytes.NewReader(data))
	if err != nil {
		return err
	}

	err = b.Tx.Bucket(BKTTxs).Delete(txId.Bytes())
	if err != nil {
		return err
	}

	b.delTxs = append(b.delTxs, &tx)
	return nil
}

func (b *txsBatch) DelAll(height uint32) error {
	b.Lock()
	defer b.Unlock()

	var key [4]byte
	binary.LittleEndian.PutUint32(key[:], height)
	data := b.Tx.Bucket(BKTHeightTxs).Get(key[:])

	var txMap = make(map[common.Uint256]uint32)
	err := gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
	if err != nil {
		return err
	}

	txBucket := b.Tx.Bucket(BKTTxs)
	for txId := range txMap {
		if err := txBucket.Delete(txId.Bytes()); err != nil {
			return err
		}
	}

	return b.Tx.Bucket(BKTHeightTxs).Delete(key[:])
}

func (b *txsBatch) Commit() error {
	b.Lock()
	defer b.Unlock()

	index := b.Tx.Bucket(BKTHeightTxs)

	// Put height index for added transactions.
	if len(b.addTxs) > 0 {
		groups := groupByHeight(b.addTxs)
		for height, txs := range groups {
			var key [4]byte
			binary.LittleEndian.PutUint32(key[:], height)
			data := index.Get(key[:])

			var txMap = make(map[common.Uint256]uint32)
			// Ignore decode error, could be first adding.
			gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)

			for _, tx := range txs {
				txMap[tx.Hash] = height
			}

			var buf = new(bytes.Buffer)
			if err := gob.NewEncoder(buf).Encode(txMap); err != nil {
				return err
			}

			return index.Put(key[:], buf.Bytes())
		}
	}

	// Update height index for deleted transactions.
	if len(b.delTxs) > 0 {
		groups := groupByHeight(b.delTxs)
		for height, txs := range groups {
			var key [4]byte
			binary.LittleEndian.PutUint32(key[:], height)
			data := index.Get(key[:])

			var txMap = make(map[common.Uint256]uint32)
			err := gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
			if err != nil {
				return err
			}

			for _, tx := range txs {
				delete(txMap, tx.Hash)
			}

			var buf = new(bytes.Buffer)
			if err = gob.NewEncoder(buf).Encode(txMap); err != nil {
				return err
			}

			return index.Put(key[:], buf.Bytes())
		}
	}

	return b.Tx.Commit()
}

func (b *txsBatch) Rollback() error {
	b.Lock()
	defer b.Unlock()
	return b.Tx.Rollback()
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
