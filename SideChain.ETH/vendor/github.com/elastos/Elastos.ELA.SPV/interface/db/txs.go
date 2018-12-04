package db

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/boltdb/bolt"
)

type Txs interface {
	Put(tx *StoreTx) error
	Get(txId *common.Uint256) (*StoreTx, error)
	GetIds(height uint32) ([]*common.Uint256, error)
}

var (
	BKTTxs       = []byte("Txs")
	BKTHeightTxs = []byte("HeightTxs")
)

type TxStore struct {
	*sync.RWMutex
	*bolt.DB
}

func NewTxsDB(db *bolt.DB) (*TxStore, error) {
	store := new(TxStore)
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

func (t *TxStore) Put(txn *StoreTx) (err error) {
	t.Lock()
	defer t.Unlock()

	return t.Update(func(tx *bolt.Tx) error {
		buf := new(bytes.Buffer)
		if err = txn.Serialize(buf); err != nil {
			return err
		}

		if err = tx.Bucket(BKTTxs).Put(txn.Hash().Bytes(), buf.Bytes()); err != nil {
			return err
		}

		var key [4]byte
		binary.LittleEndian.PutUint32(key[:], txn.Height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		var txMap = make(map[common.Uint256]uint32)
		gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)

		txMap[txn.Hash()] = txn.Height

		buf = new(bytes.Buffer)
		if err = gob.NewEncoder(buf).Encode(txMap); err != nil {
			return err
		}

		return tx.Bucket(BKTHeightTxs).Put(key[:], buf.Bytes())
	})
}

func (t *TxStore) Get(hash *common.Uint256) (txn *StoreTx, err error) {
	t.RLock()
	defer t.RUnlock()

	err = t.View(func(tx *bolt.Tx) error {
		data := tx.Bucket(BKTTxs).Get(hash.Bytes())
		txn = new(StoreTx)
		return txn.Deserialize(bytes.NewReader(data))
	})

	return txn, err
}

func (t *TxStore) GetIds(height uint32) (txIds []*common.Uint256, err error) {
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
