package store

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
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
		binary.LittleEndian.PutUint32(key[:], txn.Height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		var txMap = make(map[common.Uint256]uint32)
		gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)

		txMap[txn.Hash] = txn.Height

		buf = new(bytes.Buffer)
		if err = gob.NewEncoder(buf).Encode(txMap); err != nil {
			return err
		}

		return tx.Bucket(BKTHeightTxs).Put(key[:], buf.Bytes())
	})
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
		binary.LittleEndian.PutUint32(key[:], height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		var txMap = make(map[common.Uint256]uint32)
		err = gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
		if err != nil {
			return err
		}

		txIds = make([]*common.Uint256, 0, len(txMap))
		for hash := range txMap {
			var txId common.Uint256
			copy(txId[:], hash[:])
			txIds = append(txIds, &txId)
		}
		return nil
	})

	return txIds, err
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
		binary.LittleEndian.PutUint32(key[:], txn.Height)
		data = tx.Bucket(BKTHeightTxs).Get(key[:])

		var txMap = make(map[common.Uint256]uint32)
		err = gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
		if err != nil {
			return err
		}
		delete(txMap, *txId)

		var buf = new(bytes.Buffer)
		if err = gob.NewEncoder(buf).Encode(txMap); err != nil {
			return err
		}

		return tx.Bucket(BKTHeightTxs).Put(key[:], buf.Bytes())
	})
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
