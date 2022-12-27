package store

import (
	"bytes"
	"encoding/binary"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
	dbutil "github.com/syndtr/goleveldb/leveldb/util"
)

var (
	BKTTxs       = []byte("T")
	BKTHeightTxs = []byte("H")
	BKTForkTxs   = []byte("F")
)

// Ensure txs implement Txs interface.
var _ Txs = (*txs)(nil)

type txs struct {
	sync.RWMutex
	db *leveldb.DB
}

func NewTxs(db *leveldb.DB) *txs {
	return &txs{db: db}
}

func (t *txs) Put(txn *util.Tx) error {
	t.Lock()
	defer t.Unlock()

	buf := new(bytes.Buffer)
	if err := txn.Serialize(buf); err != nil {
		return err
	}

	batch := new(leveldb.Batch)
	batch.Put(toKey(BKTTxs, txn.Hash.Bytes()...), buf.Bytes())

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], txn.Height)
	data, _ := t.db.Get(toKey(BKTHeightTxs, key[:]...), nil)
	batch.Put(toKey(BKTHeightTxs, key[:]...), putTxId(data, &txn.Hash))

	return t.db.Write(batch, nil)
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

	data, err := t.db.Get(toKey(BKTTxs, hash.Bytes()...), nil)
	if err != nil {
		return nil, err
	}
	txn = new(util.Tx)
	err = txn.Deserialize(bytes.NewReader(data))
	if err != nil {
		return nil, err
	}

	return txn, err
}

func (t *txs) GetIds(height uint32) (txIds []*common.Uint256, err error) {
	t.RLock()
	defer t.RUnlock()

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], height)
	data, _ := t.db.Get(toKey(BKTHeightTxs, key[:]...), nil)
	return getTxIds(data), nil
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
		copy(txId[:], data[i*32:(i+1)*32])
		txIds = append(txIds, &txId)
	}

	return txIds
}

func (t *txs) PutForkTxs(txs []*util.Tx, hash *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	buf := new(bytes.Buffer)
	if err := common.WriteUint16(buf, uint16(len(txs))); err != nil {
		return err
	}
	for _, tx := range txs {
		if err := tx.Serialize(buf); err != nil {
			return err
		}
	}
	return t.db.Put(toKey(BKTForkTxs, hash.Bytes()...), buf.Bytes(), nil)
}

func (t *txs) GetForkTxs(hash *common.Uint256) ([]*util.Tx, error) {
	t.RLock()
	defer t.RUnlock()

	data, err := t.db.Get(toKey(BKTForkTxs, hash.Bytes()...), nil)
	if err != nil {
		return nil, err
	}
	buf := bytes.NewReader(data)
	count, err := common.ReadUint16(buf)
	if err != nil {
		return nil, err
	}

	txs := make([]*util.Tx, count)
	for i := range txs {
		var utx util.Tx
		if err := utx.Deserialize(buf); err != nil {
			return nil, err
		}
		txs[i] = &utx
	}
	return txs, nil
}

func (t *txs) GetAll() (txs []*util.Tx, err error) {
	t.RLock()
	defer t.RUnlock()

	it := t.db.NewIterator(dbutil.BytesPrefix(BKTTxs), nil)
	defer it.Release()
	for it.Next() {
		var txn util.Tx
		err := txn.Deserialize(bytes.NewReader(it.Value()))
		if err != nil {
			return nil, err
		}
		txs = append(txs, &txn)
	}
	return txs, err
}

func (t *txs) Del(txId *common.Uint256) error {
	t.Lock()
	defer t.Unlock()

	var txn util.Tx
	data, err := t.db.Get(toKey(BKTTxs, txId.Bytes()...), nil)
	if err != nil {
		return err
	}

	if err := txn.Deserialize(bytes.NewReader(data)); err != nil {
		return err
	}

	var key [4]byte
	binary.BigEndian.PutUint32(key[:], txn.Height)
	data, _ = t.db.Get(toKey(BKTHeightTxs, key[:]...), nil)

	batch := new(leveldb.Batch)
	batch.Delete(toKey(BKTTxs, txId.Bytes()...))
	batch.Put(toKey(BKTHeightTxs, key[:]...), delTxId(data, &txn.Hash))

	return t.db.Write(batch, nil)
}

func delTxId(data []byte, hash *common.Uint256) []byte {
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
		copy(txId[:], data[i*32:(i+1)*32])
		if txId.IsEqual(*hash) {
			data = append(data[0:i*32], data[(i+1)*32:]...)
			break
		}
	}
	var buf [2]byte
	binary.BigEndian.PutUint16(buf[:], count-1)

	return append(buf[:], data...)
}

func (t *txs) Batch() TxsBatch {
	return &txsBatch{DB: t.db, Batch: new(leveldb.Batch)}
}

func (t *txs) Clear() error {
	t.Lock()
	defer t.Unlock()

	it := t.db.NewIterator(dbutil.BytesPrefix(BKTTxs), nil)
	batch := new(leveldb.Batch)
	for it.Next() {
		batch.Delete(it.Key())
	}
	it.Release()

	it = t.db.NewIterator(dbutil.BytesPrefix(BKTHeightTxs), nil)
	for it.Next() {
		batch.Delete(it.Key())
	}
	it.Release()

	return t.db.Write(batch, nil)
}

func (t *txs) Close() error {
	t.Lock()
	return nil
}
