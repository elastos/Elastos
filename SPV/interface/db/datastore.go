package db

import (
	"bytes"
	"encoding/binary"
	"encoding/gob"
	"github.com/boltdb/bolt"
	"sync"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/core"
)

type DataStore interface {
	Addrs() Addrs
	Txs() Txs
	Outpoints() Outpoints
	Proofs() Proofs
	Rollback(height uint32) error
	Reset() error
	Close()
}

type DataStoreImpl struct {
	*sync.RWMutex
	*bolt.DB
	addrs     *AddrStore
	txs       *TxStore
	outpoints *OutpointStore
	proofs    *ProofStore
}

func NewDataStore() (*DataStoreImpl, error) {
	db, err := bolt.Open("data_store.bin", 0644, &bolt.Options{InitialMmapSize: 5000000})
	if err != nil {
		return nil, err
	}
	store := new(DataStoreImpl)
	store.RWMutex = new(sync.RWMutex)
	store.DB = db

	store.addrs, err = NewAddrsDB(db)
	if err != nil {
		return nil, err
	}

	store.txs, err = NewTxsDB(db)
	if err != nil {
		return nil, err
	}

	store.outpoints, err = NewOutpointDB(db)
	if err != nil {
		return nil, err
	}

	store.proofs, err = NewProofsDB(db)
	if err != nil {
		return nil, err
	}

	return store, nil
}

func (d *DataStoreImpl) Addrs() Addrs {
	return d.addrs
}

func (d *DataStoreImpl) Txs() Txs {
	return d.txs
}

func (d *DataStoreImpl) Outpoints() Outpoints {
	return d.outpoints
}

func (d *DataStoreImpl) Proofs() Proofs {
	return d.proofs
}

func (d *DataStoreImpl) Rollback(height uint32) error {
	d.Lock()
	defer d.Unlock()

	return d.Update(func(tx *bolt.Tx) error {
		var key [4]byte
		binary.LittleEndian.PutUint32(key[:], height)
		data := tx.Bucket(BKTHeightTxs).Get(key[:])

		var txMap = make(map[common.Uint256]uint32)
		err := gob.NewDecoder(bytes.NewReader(data)).Decode(&txMap)
		if err != nil {
			return err
		}

		for hash := range txMap {
			var txn StoreTx
			data := tx.Bucket(BKTTxs).Get(hash.Bytes())
			if err = txn.Deserialize(bytes.NewReader(data)); err != nil {
				return err
			}
			for index, output := range txn.Outputs {
				if d.addrs.GetFilter().ContainAddr(output.ProgramHash) {
					outpoint := core.NewOutPoint(txn.Hash(), uint16(index)).Bytes()
					tx.Bucket(BKTOps).Delete(outpoint)
				}
			}
			if err = tx.Bucket(BKTTxs).Delete(hash.Bytes()); err != nil {
				return err
			}
		}
		return nil
	})
}

func (d *DataStoreImpl) Reset() error {
	d.Lock()
	defer d.Unlock()

	return d.Update(func(tx *bolt.Tx) error {
		tx.DeleteBucket(BKTAddrs)
		tx.DeleteBucket(BKTTxs)
		tx.DeleteBucket(BKTHeightTxs)
		tx.DeleteBucket(BKTOps)
		tx.DeleteBucket(BKTProofs)
		return nil
	})
}

// Close db
func (d *DataStoreImpl) Close() {
	d.Lock()
	d.addrs.Lock()
	d.txs.Lock()
	d.outpoints.Lock()
	d.proofs.Lock()
	d.DB.Close()
}
