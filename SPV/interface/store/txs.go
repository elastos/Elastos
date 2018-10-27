package store

import (
	"bytes"
	"encoding/binary"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

var (
	BKTTxs       = []byte("Txs")
	BKTHeightTxs = []byte("HeightTxs")
)

// Ensure txs implement Txs interface.
var _ Txs = (*txs)(nil)

type txs struct {
	*sync.RWMutex
	*bolt.DB
}

func NewTxs(db *bolt.DB) (*txs, error) {
	store := new(txs)
	store.RWMutex = new(sync.RWMutex)
	store.DB = db

	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTTxs)
		if err != nil {
			return err
		}
		_, err = btx.CreateBucketIfNotExists(BKTHeightTxs)
		if err != nil {
			return err
		}
		return nil
	})

	return store, nil
}

func (t *txs) Put(txn *util.Tx) (err error) {
	t.Lock()
	defer t.Unlock()

	return t.Update(func(tx *bolt.Tx) error {
		buf := new(bytes.Buffer)
		if err = txn.Serialize(buf); err != nil {
			return err
		}

		if err = tx.Bucket(BKTTxs).Put(txn.Hash.Bytes(), buf.Bytes()); err != nil {
			return err
		}

		var key [4]byte
		binary.BigEndian.PutUint32(key[:], txn.Height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		data = putTxId(data, &txn.Hash)

		return tx.Bucket(BKTHeightTxs).Put(key[:], data)
	})
}

func putTxId(data []byte, txId *common.Uint256) []byte {
	// Get tx count
	var count uint16
	if len(data) == 0 {
		data = append(data, 0, 0)
	} else {
		count = binary.BigEndian.Uint16(data[0:2])
	}

	data = append(data, txId[:]...)
	binary.BigEndian.PutUint16(data[0:2], count+1)
	return data
}

func (t *txs) Get(hash *common.Uint256) (txn *util.Tx, err error) {
	t.RLock()
	defer t.RUnlock()

	err = t.View(func(tx *bolt.Tx) error {
		data := tx.Bucket(BKTTxs).Get(hash.Bytes())
		txn = new(util.Tx)
		return txn.Deserialize(bytes.NewReader(data))
	})

	return txn, err
}

func (t *txs) GetIds(height uint32) (txIds []*common.Uint256, err error) {
	t.RLock()
	defer t.RUnlock()

	err = t.View(func(tx *bolt.Tx) error {
		var key [4]byte
		binary.BigEndian.PutUint32(key[:], height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		txIds = getTxIds(data)

		return nil
	})

	return txIds, err
}

func getTxIds(data []byte) (txIds []*common.Uint256) {
	// Get tx count
	var count uint16
	if len(data) == 0 {
		return nil
	} else {
		count = binary.BigEndian.Uint16(data[0:2])
	}

	data = data[2:]
	for i := uint16(0); i < count; i++ {
		var txId common.Uint256
		copy(txId[:], data[i*common.UINT256SIZE : (i+1)*common.UINT256SIZE])
		txIds = append(txIds, &txId)
	}

	return txIds
}

func (t *txs) GetAll() (txs []*util.Tx, err error) {
	t.RLock()
	defer t.RUnlock()

	err = t.View(func(tx *bolt.Tx) error {
		return tx.Bucket(BKTTxs).ForEach(func(k, v []byte) error {
			var txn util.Tx
			err := txn.Deserialize(bytes.NewReader(v))
			if err != nil {
				return err
			}
			txs = append(txs, &txn)
			return nil
		})
	})

	return txs, err
}

func (t *txs) Del(txId *common.Uint256) (err error) {
	t.Lock()
	defer t.Unlock()

	return t.DB.Update(func(tx *bolt.Tx) error {
		var txn util.Tx
		data := tx.Bucket(BKTTxs).Get(txId.Bytes())
		err := txn.Deserialize(bytes.NewReader(data))
		if err != nil {
			return err
		}

		var key [4]byte
		binary.BigEndian.PutUint32(key[:], txn.Height)
		data = tx.Bucket(BKTHeightTxs).Get(key[:])

		data = delTxId(data, &txn.Hash)

		return tx.Bucket(BKTHeightTxs).Put(key[:], data)
	})
}

func delTxId(data []byte, hash *common.Uint256) []byte{
	// Get tx count
	var count uint16
	if len(data) == 0 {
		return nil
	} else {
		count = binary.BigEndian.Uint16(data[0:2])
	}

	data = data[2:]
	for i := uint16(0); i < count; i++ {
		var txId common.Uint256
		copy(txId[:],data[i*common.UINT256SIZE : (i+1)*common.UINT256SIZE])
		if txId.IsEqual(*hash) {
			data = append(data[0:i*common.UINT256SIZE], data[(i+1)*common.UINT256SIZE:]...)
			break
		}
	}
	var buf [2]byte
	binary.BigEndian.PutUint16(buf[:], count-1)

	return append(buf[:], data...)
}

func (t *txs) Batch() TxsBatch {
	tx, err := t.DB.Begin(true)
	if err != nil {
		panic(err)
	}

	return &txsBatch{Tx: tx}
}

func (t *txs) Clear() error {
	t.Lock()
	defer t.Unlock()

	return t.DB.Update(func(tx *bolt.Tx) error {
		if err := tx.DeleteBucket(BKTTxs); err != nil {
			return err
		}

		if err := tx.DeleteBucket(BKTHeightTxs); err != nil {
			return err
		}

		return nil
	})
}

func (t *txs) Close() error {
	t.Lock()
	return nil
}
