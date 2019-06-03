package store

import (
	"bytes"
	"encoding/binary"
	"path/filepath"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
	"github.com/syndtr/goleveldb/leveldb/errors"
	"github.com/syndtr/goleveldb/leveldb/util"
)

// Ensure dposStore implement the DPOSStore interface.
var _ DPOSStore = (*dposStore)(nil)

var (
	BKTMIndex  = []byte("M_INDEX")
	BKTPubKey  = []byte("M_PUBKEY")
	BKTProdID  = []byte("M_PRODID")
	BKTBlocks  = []byte("M_BLOCKS")
	BKTArchive = []byte("M_ARCHIV")
	BKTMHeight = []byte("M_HEIGHT")
)

// dposStore is the implementation of DPOSStore interface.
type dposStore struct {
	dataStore
	mtx sync.Mutex
	db  *leveldb.DB
}

// increaseIndex returns a increased index.
func (s *dposStore) increaseIndex(batch *leveldb.Batch) (index []byte) {
	index, err := s.db.Get(BKTMIndex, nil)
	if err == errors.ErrNotFound {
		// Initial index value uint32(0).
		index = []byte{0x0, 0x0, 0x0, 0x0}
		batch.Put(BKTMIndex, index)
		return index
	}
	binary.BigEndian.PutUint32(index, binary.BigEndian.Uint32(index)+1)
	batch.Put(BKTMIndex, index)
	return index
}

// Mapping save a owner public key and side producer ID mapping into database.
// The mapping will be activity after 6 blocks confirmation.
func (s *dposStore) Mapping(ownerPublicKey, sideProducerID []byte, height uint32) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	var batch = &leveldb.Batch{}
	var index = s.increaseIndex(batch)
	var registerHeight = make([]byte, 4)
	binary.BigEndian.PutUint32(registerHeight, height)
	batch.Put(toKey(BKTPubKey, ownerPublicKey...), index)
	batch.Put(toKey(BKTProdID, index...),
		append(registerHeight, sideProducerID...))
	return s.db.Write(batch, nil)
}

// Archive saves the current producers on the specific height.
func (s *dposStore) Archive(currentProducers [][]byte, height uint32,
	blockHash *common.Uint256) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	// Find side producers indices.
	var buf = new(bytes.Buffer)
	var batch = &leveldb.Batch{}
	var archiveHeight = make([]byte, 4)
	binary.BigEndian.PutUint32(archiveHeight, height)
	for _, pubKey := range currentProducers {
		index, err := s.db.Get(toKey(BKTPubKey, pubKey...), nil)
		if err != nil {
			continue
		}
		value, err := s.db.Get(toKey(BKTProdID, index...), nil)
		if err != nil {
			continue
		}
		if binary.BigEndian.Uint32(value[:4])+6 > height {
			continue
		}
		_, err = buf.Write(index)
		if err != nil {
			return err
		}
	}
	batch.Put(toKey(BKTArchive, archiveHeight...), buf.Bytes())

	if blockHash != nil {
		// Archive block hash on this height.
		batch.Put(toKey(BKTBlocks, archiveHeight...), blockHash[:])

		// Update last archive height.
		batch.Put(BKTMHeight, archiveHeight)
	}

	// Write changes into database.
	return s.db.Write(batch, nil)
}

// GetBlockHash returns the block hash on of the specific height.
func (s *dposStore) GetBlockHash(height uint32) (*common.Uint256, error) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	var blockHeight = make([]byte, 4)
	binary.BigEndian.PutUint32(blockHeight, height)

	blockHash, err := s.db.Get(toKey(BKTBlocks, blockHeight...), nil)
	if err != nil {
		return nil, err
	}
	return common.Uint256FromBytes(blockHash)
}

// GetProducers returns the side producer IDs who are participated in block
// producing.
func (s *dposStore) GetProducers(height uint32) ([][]byte, error) {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	var archiveHeight = make([]byte, 4)
	binary.BigEndian.PutUint32(archiveHeight, height)
	archive, err := s.db.Get(toKey(BKTArchive, archiveHeight...), nil)
	if err != nil {
		return nil, err
	}

	var producers = make([][]byte, 0, len(archive)/4)
	for i := 0; i < len(archive); i += 4 {
		value, err := s.db.Get(toKey(BKTProdID, archive[i:i+4]...), nil)
		if err != nil {
			return nil, err
		}
		producers = append(producers, value[4:])
	}

	return producers, nil
}

// Rollback restore the Mapping database to previous height.
func (s *dposStore) Rollback() error {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	// Get last archive height.
	lastHeight, err := s.db.Get(BKTMHeight, nil)
	if err != nil {
		return err
	}
	height := binary.BigEndian.Uint32(lastHeight)

	batch := &leveldb.Batch{}
	// Delete last archive.
	batch.Delete(toKey(BKTArchive, lastHeight...))

	// Delete registered producer IDs on last height.
	i := s.db.NewIterator(util.BytesPrefix(BKTProdID), nil)
	defer i.Release()
	for i.Next() {
		registerHeight := i.Value()[:4]
		if !bytes.Equal(lastHeight, registerHeight) {
			continue
		}
		batch.Delete(i.Key())
	}

	// Set last archive height to previous.
	binary.BigEndian.PutUint32(lastHeight, height-1)
	batch.Put(BKTMHeight, lastHeight)
	return s.db.Write(batch, nil)
}

// Close database.
func (s *dposStore) Close() error {
	s.mtx.Lock()
	return s.dataStore.Close()
}

// NewDPOSStore creates a new DPOSStore instance.
func NewDPOSStore(dataDir string) (DPOSStore, error) {
	db, err := leveldb.OpenFile(filepath.Join(dataDir, "store"), nil)
	if err != nil {
		return nil, err
	}
	if err != nil {
		return nil, err
	}

	addrs, err := NewAddrs(db)
	if err != nil {
		return nil, err
	}

	return &dposStore{
		dataStore: dataStore{
			db:    db,
			addrs: addrs,
			txs:   NewTxs(db),
			ops:   NewOps(db),
			que:   NewQue(db),
		},
		db: db,
	}, nil
}
